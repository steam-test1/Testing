#pragma once

#include <dbutil/Datastore.h>
#include <functional>
#include <optional>
#include <platform.h>

namespace blt
{
	namespace plugins
	{
		// Implements some stuff exactly like pd2hook::tweaker::dbhook but for native plugins

		namespace dbhook
		{
			class DBTargetFile
			{
			  public:
				blt::idfile id;
				std::function<void(std::vector<uint8_t>*)> replacer = nullptr;
				std::optional<std::string> plain_file;
				blt::idfile direct_bundle = blt::idfile();
				bool fallback_mode = false;

				explicit DBTargetFile(blt::idfile id) : id(id)
				{
				}
				void DBTargetFile::setReplacer(std::function<void(std::vector<uint8_t>*)> replacer);
				void DBTargetFile::setFallback(bool fallback);
				void DBTargetFile::setPlainFile(std::string path);
				void DBTargetFile::setDirectBundle(blt::idfile bundle);
				void DBTargetFile::setDirectBundle(std::string name, std::string ext);
				bool DBTargetFile::hasReplacer();
				bool DBTargetFile::hasPlainFile();
				bool DBTargetFile::hasDirectBundle();
				void DBTargetFile::clear_sources();
			};

			bool hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore, int64_t* out_pos,
			                     int64_t* out_len, std::string& out_name, bool fallback_mode);
			void registerAssetHook(std::string name, std::string ext, bool fallback, DBTargetFile** out_target);

			bool fileExists(std::string name, std::string ext);
			std::vector<uint8_t> findFile(std::string name, std::string ext);
		} // namespace dbhook
	} // namespace plugins
} // namespace blt