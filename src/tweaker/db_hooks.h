//
// Created by ZNix on 23/11/2020.
//

#pragma once

#include <dbutil/Datastore.h>
#include <platform.h>
#include <wren.hpp>
#include <optional>

namespace pd2hook::tweaker::dbhook
{
    struct FileData {
        uint8_t* data;
        size_t size;
        unsigned long long name;
        unsigned long long ext;
    };

    typedef void (*db_file_replacer_t)(FileData*);
	class DBTargetFile
	{
	  public:
		blt::idfile id;

		/** If true, this asset will only be loaded if the default asset with this name/ext does not exist. */
		bool fallback = false;

		/** The path to the file to load and use for this asset */
		std::optional<std::string> plain_file;

		/** The ID of the in-bundle asset to use */
		blt::idfile direct_bundle = blt::idfile();

		/** The handle to a Wren object to run the loading callback on */
		WrenHandle* wren_loader_obj = nullptr;

		/** To handle native plugin asset replacement */
        db_file_replacer_t replacer = nullptr;

		explicit DBTargetFile(blt::idfile id) : id(id)
		{
		}

        void SetReplacer(db_file_replacer_t replacer);
        void SetFallback(bool fallback);
        void SetPlainFile(std::string path);
        void SetDirectBundle(blt::idfile bundle);
        void SetDirectBundle(std::string name, std::string ext);
        bool HasReplacer();
        bool HasPlainFile();
        bool HasDirectBundle();
        bool HasWrenLoader();

		void clear_sources();
	};

	WrenForeignMethodFn bind_dbhook_method(WrenVM* vm, const char* module, const char* class_name_s, bool is_static,
	                                       const char* signature_c);

	WrenForeignClassMethods bind_dbhook_class(WrenVM* vm, const char* module, const char* class_name);

	// Return true if the asset was found and the resulting datastore has been set, false otherwise.
	bool hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore, int64_t* out_pos,
	                     int64_t* out_len, std::string& out_name, bool fallback_mode);

    void register_asset_hook(std::string name, std::string ext, bool fallback, DBTargetFile** out_target);

    bool file_exists(std::string name, std::string ext);

    FileData find_file(std::string name, std::string ext);
} // namespace pd2hook::tweaker::dbhook
