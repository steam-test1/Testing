#include "native_db_hooks.h"

#include <dbutil/DB.h>

#include <map>
#include <util/util.h>
#include <vector>
#include <fstream>

using blt::db::DieselDB;
using blt::db::DslFile;
using blt::plugins::dbhook::DBTargetFile;

void DBTargetFile::setReplacer(std::function<void(std::vector<uint8_t>*)> replacer)
{
    this->replacer = replacer;
}

void DBTargetFile::setFallback(bool fallback)
{
    this->fallback_mode = fallback;
}

static std::map<blt::idfile, std::shared_ptr<DBTargetFile>> overriddenFiles;

void blt::plugins::dbhook::registerAssetHook(std::string name, std::string ext, bool fallback,
                                             DBTargetFile** out_target)
{
	blt::idstring nameids = blt::idstring_hash(name.c_str());
	blt::idstring extids = blt::idstring_hash(ext.c_str());

	blt::idfile file(nameids, extids);

	if (overriddenFiles.count(file))
	{
		std::string message = "Duplicate asset registration for " + name + "." + ext;
		PD2HOOK_LOG_ERROR(message.c_str());
        return;
	}

	auto entry = std::make_shared<DBTargetFile>(file);
	overriddenFiles[file] = entry;
	*out_target = entry.get();

    std::string msg = "Registered asset hook for " + name + "." + ext;
    PD2HOOK_LOG_LOG(msg.c_str());
}

bool blt::plugins::dbhook::hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore,
                                           int64_t* out_pos, int64_t* out_len, std::string& out_name,
                                           bool fallback_mode)
{
	*out_datastore = nullptr;
	*out_pos = 0;
	*out_len = 0;

	auto filePtr = overriddenFiles.find(asset_file);

    if (filePtr == overriddenFiles.end())
        return false;

	DBTargetFile& target = *filePtr->second;

	if (target.fallback_mode && !fallback_mode)
		return false;

    // TODO: implement this just like in wren where you can set a plain file or a direct bundle

    DslFile* file = DieselDB::Instance()->Find(target.id.name,  target.id.ext);

    if (!file) {
        char buff[1024];
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff) - 1, "Failed to open hooked asset file " IDPFP " while loading " IDPFP,
                    target.id.name, target.id.ext, asset_file.name, asset_file.ext);
        PD2HOOK_LOG_ERROR(buff);

#ifdef _WIN32
        MessageBox(nullptr, "Failed to load hooked asset - file not found. See log for more information.",
                    "Native Plugin Error", MB_OK);
        ExitProcess(1);
#else
        abort();
#endif
    }

    std::ifstream stream(file->bundle->path, std::ios::binary);
    if (stream.fail())
	{
        std::string message = "Failed to open bundle file containing the asset - " + file->bundle->path;
        PD2HOOK_LOG_ERROR(message.c_str());

#ifdef _WIN32
        MessageBox(nullptr, "Failed to load hooked asset - failed to open bundle file containing the asset. See log for more information.",
                    "Native Plugin Error", MB_OK);
        ExitProcess(1);
#else
        abort();
#endif
	}

    std::vector<uint8_t> data = file->ReadContents(stream);

    target.replacer(&data);

    std::string str(data.begin(), data.end());

    auto* ds = new BLTStringDataStore(str);

    *out_datastore = ds;
    *out_len = ds->size();

    return true;
}