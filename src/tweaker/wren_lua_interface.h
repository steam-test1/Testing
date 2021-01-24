//
// Created by znix on 24/01/2021.
//

#pragma once

#include <lua.h>

namespace pd2hook::tweaker::lua_io
{
	void register_lua_functions(lua_State* L);

	// Don't include wren.h if unneeded
#ifdef wren_h
	WrenForeignMethodFn bind_wren_lua_method(WrenVM* vm, const char* module, const char* class_name, bool is_static,
	                                         const char* signature);
#endif

} // namespace pd2hook::tweaker::lua_io
