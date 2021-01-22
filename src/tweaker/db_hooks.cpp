//
// Created by ZNix on 23/11/2020.
//

#include "db_hooks.h"

#include <string>

bool pd2hook::tweaker::dbhook::hook_asset_load(const blt::idfile& asset_file, BLTAbstractDataStore** out_datastore,
                                               int64_t* out_pos, int64_t* out_len, std::string& out_name,
                                               bool fallback_mode)
{
	// First zero everything
	*out_datastore = nullptr;
	*out_pos = 0;
	*out_len = 0;

	// For now, don't hook anything
	// TODO implement
	return false;
}
