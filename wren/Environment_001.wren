// Functions mods can use to probe their enviornment

class Environment {
    // Gets the path to the mod's directory, eg mods/mymod. Useful for loading assets.
    // Note this is queried with *native magic*, so unlike ModPath in Lua it depends
    // on who's calling it - thus you don't have to cache it's result value.
    foreign static mod_directory
}
