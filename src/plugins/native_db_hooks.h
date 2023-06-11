#pragma once

#include <dbutil/Datastore.h>
#include <platform.h>
#include <functional>

namespace blt {
    namespace plugins {
        // Implements some stuff exactly like pd2hook::tweaker::dbhook but for native plugins
        namespace dbhook {
            class DBTargetFile {
                public:
                    blt::idfile id;
                    std::function<void(std::vector<uint8_t>*)> replacer;
                    bool fallback_mode = false;

                    explicit DBTargetFile(blt::idfile id) : id(id)
                    {
                    }

                    void setReplacer(std::function<void(std::vector<uint8_t>*)> replacer);
                    void setFallback(bool fallback);
            };

            bool hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore, int64_t* out_pos, int64_t* out_len, std::string& out_name, bool fallback_mode);
            void registerAssetHook(std::string name, std::string ext, bool fallback, DBTargetFile** out_target);
        }
    }
}