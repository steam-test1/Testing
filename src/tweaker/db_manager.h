//
// Created by ZNix on 23/11/2020.
//

#pragma once

#include <dbutil/Datastore.h>
#include <platform.h>

namespace pd2hook::tweaker::dbhook
{
	// Return true if the asset was found and the resulting datastore has been set, false otherwise.
	bool hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore, int64_t* out_pos,
	                     int64_t* out_len, std::string& out_name, bool fallback_mode);
} // namespace pd2hook::tweaker::dbhook
