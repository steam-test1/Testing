#pragma once

#include <stdint.h>

#ifdef _WIN32
#include "../platforms/w32/signatures/sigdef.h"
#else
#include "../platforms/linux/include/lua.hh"
#endif

inline uint64_t luaX_toidstring(lua_State *L, int index)
{
	return *(uint64_t*) lua_touserdata(L, index);
}
