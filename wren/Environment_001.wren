// Functions mods can use to probe their enviornment

class Environment {
    // Gets the path to the mod's directory, eg mods/mymod. Useful for loading assets.
    // Note this is queried with *native magic*, so unlike ModPath in Lua it depends
    // on who's calling it - thus you don't have to cache it's result value.
    foreign static mod_directory

    // When depth is 0 this is the same as mod_directory. When depth is 1 this finds the
    // mod directory of whatever's calling the function calling this, and so on.
    // This is particularly useful inside the basemod to separate stuff by mod, but care
    // should be used when using it as it can lead to non-obvious results for code a few
    // layers up in the callstack.
    foreign static mod_directory_at_depth(depth)
}
