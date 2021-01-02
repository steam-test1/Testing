#include "DB.h"

#include <util/util.h>

#include <fstream>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <assert.h>

#include <stdio.h>

namespace fs = std::filesystem;
using namespace blt::db;

static_assert(sizeof(void*) == sizeof(intptr_t));
using pos_t = std::ios::pos_type;
using FileList = std::vector<DslFile>&;
using FileMap = std::map<std::pair<idstring, idstring>, DslFile*>&;

struct dsl_Vector
{
	unsigned int size;
	unsigned int capacity;
	intptr_t contents_ptr;
	void *allocator;
};

static void skipVector(std::istream &in)
{
	in.seekg(sizeof(dsl_Vector), std::ios::cur);
}

template<typename T>
static std::vector<T> loadVector(std::istream &in, int offset, dsl_Vector &vec)
{
	std::vector<T> data;
	data.resize(vec.size);

	// Read the vector contents
	pos_t pos = in.tellg();
	in.seekg(vec.contents_ptr + offset);
	in.read((char*)data.data(), sizeof(T)*data.size());
	in.seekg(pos);

	return data;
}

template<typename T>
static std::vector<T> loadVector(std::istream &in, int offset)
{
	// Read the vector metadata
	dsl_Vector vec = {0};
	in.read((char*)&vec, sizeof(vec));

	return loadVector<T>(in, offset, vec);
}

DieselDB* DieselDB::Instance()
{
	static DieselDB instance;
	return &instance;
}

static void loadPackageHeader(DieselBundle *bundle, FileList);
static void loadBundleHeader(std::string filename, FileList);

DieselDB::DieselDB()
{
	std::ifstream in;
	in.exceptions(std::ios::failbit | std::ios::badbit);
	in.open("assets/bundle_db.blb", std::ios::binary);

	// Skip a pointer - vtable or allocator probably?
	in.seekg(sizeof(void*), std::ios::cur);

	// Skip the languages vector for now
	skipVector(in);

	// Sortmap
	in.seekg(sizeof(void*) * 2, std::ios::cur);

	// Files
	struct MiniFile
	{
		idstring type;
		idstring name;
		int32_t langId;
		int32_t zero_1;
		int32_t fileId;
		int32_t zero_2;
	};
	static_assert(sizeof(MiniFile) == 32); // Same on 32 and 64 bit
	std::vector<MiniFile> miniFiles = loadVector<MiniFile>(in, 0);
	filesList.resize(miniFiles.size());

	for (size_t i=0; i<miniFiles.size(); i++)
	{
		MiniFile &mini = miniFiles[i];
		// printf("File: %016llx.%016llx\n", mini.name, mini.type);
		assert(mini.zero_1 == 0);
		assert(mini.zero_2 == 0);

		// Since the file IDs form a sequence of 1 upto the file count (though not in
		// order), we can use those as indexes into our file list.
		DslFile &fi = filesList.at(mini.fileId - 1);

		fi.name = mini.name;
		fi.type = mini.type;
		fi.fileId = mini.fileId;

		// TODO language management - what do we need for this?
		fi.langId = mini.langId;

		// If it's a repeated file, the language must be different
		const auto &prev = files.find(fi.Key());
		if(prev != files.end())
		{
			assert(prev->second->langId != fi.langId);
		}

		files[fi.Key()] = &fi;
	}

	// printf("File count: %ld\n", files.size());

	// Load each of the bundle headers
	std::string suffix = "_h.bundle";
	for(const fs::directory_entry &entry : fs::directory_iterator("assets"))
	{
		if(!entry.is_regular_file()) continue;
		std::string name = entry.path().filename().string();
		if(name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0) continue;
		if(name == "all_h.bundle") continue; // all_h handling later
		if(name.size() != 25)
		{
			PD2HOOK_LOG_WARN("Invalid bundle name '" + name + "' - ignoring");
			continue;
		}

		// Find the path to the data file - chop out the '_h' bit
		std::string dataPath = entry.path().string();
		dataPath.erase(dataPath.end() - 9, dataPath.end() - 7);

		// Memory leak, not an issue since it's a small amount and the DB doesn't get unloaded anyway
		DieselBundle *bundle = new DieselBundle();
		bundle->headerPath = entry.path().string();
		bundle->path = dataPath;
		loadPackageHeader(bundle, filesList);
	}

	loadBundleHeader("assets/all_h.bundle", filesList);
}

static void loadPackageHeader(DieselBundle *bundle, FileList files)
{
	std::ifstream in;
	in.exceptions(std::ios::failbit | std::ios::badbit);
	in.open(bundle->headerPath, std::ios::binary);

	// Skip an int, the length of the header
	in.seekg(4, std::ios::cur);

	// Files
	struct FilePos
	{
		int32_t fileId;
		int32_t offset;
	};
	static_assert(sizeof(FilePos) == 8); // Same on 32 and 64 bit
	std::vector<FilePos> positions = loadVector<FilePos>(in, 4);

	DslFile *prev = nullptr;
	for(const FilePos &fp : positions)
	{
		DslFile *fi = &files.at(fp.fileId - 1);

		fi->bundle = bundle;
		fi->offset = fp.offset;
		if(prev != nullptr)
		{
			prev->length = fi->offset - prev->offset;
		}

		prev = fi;
	}

	// TODO set a length for the last file
}

static void loadBundleHeader(std::string filename, FileList files)
{
	std::ifstream in;
	in.exceptions(std::ios::failbit | std::ios::badbit);
	in.open(filename, std::ios::binary);

	// Skip an int, the length of the header
	in.seekg(4, std::ios::cur);

	struct BundleInfo
	{
		intptr_t id;
		intptr_t zero;
		dsl_Vector vec;
		intptr_t one;
	};
#if defined(__x86_64__)
	static_assert(sizeof(BundleInfo) == 48);
#else
	static_assert(sizeof(BundleInfo) == 28);
#endif

	struct ItemInfo
	{
		uint32_t fileId;
		uint32_t offset;
		uint32_t length;
	};
	static_assert(sizeof(ItemInfo) == 12); // True on 32/64 bit

	for(BundleInfo bundle : loadVector<BundleInfo>(in, 4))
	{
		assert(bundle.zero == 0);
		assert(bundle.one == 1);

		// Memory leak, not an issue since it's a small amount and the DB doesn't get unloaded anyway
		DieselBundle *dieselBundle = new DieselBundle();
		dieselBundle->headerPath = filename;
		dieselBundle->path = "assets/all_" + std::to_string(bundle.id) + ".bundle";

		for(ItemInfo item : loadVector<ItemInfo>(in, 4, bundle.vec))
		{
			DslFile *fi = &files.at(item.fileId - 1);
			fi->bundle = dieselBundle;
			fi->offset = item.offset;
			fi->length = item.length;
		}
	}
}

DslFile* DieselDB::Find(idstring name, idstring ext)
{
	auto res = files.find(std::pair<idstring, idstring>(name, ext));

	// Not found?
	if(res == files.end())
	{
		return nullptr;
	}

	return res->second;
}

BLTAbstractDataStore* DieselDB::Open(DieselBundle *bundle)
{
	// Ideally we'd cache these to avoid opening files all the time, but this is
	// pretty much what the Linux version of PD2 seems to do internally. The
	// problem here is that the data store is reference counted by dsl::Archive, so
	// caching it will be somewhat difficult.
	BLTFileDataStore *fds = BLTFileDataStore::Open(bundle->path);
	return fds;
}
