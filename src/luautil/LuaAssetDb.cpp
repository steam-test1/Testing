//
// Created by znix on 24/01/2021.
//

#include "LuaAssetDb.h"

#include <dbutil/DB.h>
#include <errno.h>
#include <fstream>
#include <inttypes.h>
#include <platform.h>
#include <string.h>
#include <util/util.h>

using blt::idstring;
using blt::db::DieselBundle;
using blt::db::DieselDB;
using blt::db::DslFile;

static idstring to_idstring(lua_State* L, int idx)
{
	const char* str = luaL_checkstring(L, idx);
	int len = strlen(str);

	// If the name is a 17-byte-long string starting with a hash, it's the plain hash
	if (len == 17 && str[0] == '#')
	{
		char* end_ptr;
		idstring value = strtoll(str, &end_ptr, 16);
		if (*end_ptr)
			luaL_error(L, "Failed to parse raw idstring '%s': parsing stopped at '%s'", str, end_ptr);
		return value;
	}

	return blt::idstring_hash(str);
}

static DslFile* find_file(lua_State* L)
{
	idstring name = to_idstring(L, 1);
	idstring ext = to_idstring(L, 2);
	// 3rd arg is an options table

	DslFile* file = DieselDB::Instance()->Find(name, ext);

	// For now, block files with languages set - we don't know which one we're getting
	// TODO let the options table specify the required language
	if (file->langId)
		return nullptr;

	return file;
}

static int ldb_load(lua_State* L)
{
	idstring name = to_idstring(L, 1);
	idstring ext = to_idstring(L, 2);

	bool optional = false; // Is it valid for the file to not exist?

	if (lua_istable(L, 3))
	{
		lua_getfield(L, 3, "optional");
		optional = lua_toboolean(L, -1);
		lua_pop(L, 1);
	}

	DslFile* file = find_file(L);

	if (!file) // Asset does not exist
	{
		if (optional)
		{
			lua_pushnil(L);
			return 1;
		}

		char msg[1024];
		snprintf(msg, sizeof(msg) - 1, "AssetDB: could not load asset " IDPFP " - not found in database", name, ext);
		luaL_error(L, msg);
		return 0; // Placate CLion's null warning thing, luaL_error never returns
	}

	errno = 0;
	try
	{
		std::ifstream fi;
		fi.exceptions(std::ios::failbit);
		fi.open(file->bundle->path, std::ios::binary);

		fi.seekg(file->offset);

		std::vector<char> data(file->length);
		fi.read(data.data(), data.size());
		lua_pushlstring(L, data.data(), data.size());
		return 1;
	}
	catch (const std::ios::failure& ex)
	{
		luaL_error(L, "Failed to read bundle: io error: %s", strerror(errno));
		return 0; // Will never happen, luaL_error does not return
	}
}

static int ldb_has(lua_State* L)
{
	DslFile* file = find_file(L);
	lua_pushboolean(L, file != nullptr);
	return 1;
}

void load_lua_asset_db(lua_State* L)
{
	// (note: ldb = Lua asset DB)
	luaL_Reg vmLib[] = {
		{"read_file", ldb_load},
		{"has_file", ldb_has},

		{nullptr, nullptr},
	};

	lua_newtable(L);
	luaL_openlib(L, nullptr, vmLib, 0);
	lua_setfield(L, -2, "asset_db");
}
