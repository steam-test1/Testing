class LuaInterface {
    // Register an object that Lua can call functions on.
    // Access to any functions with names starting with underscores will be blocked.
    foreign static register_object(lua_func_name, obj)
}
