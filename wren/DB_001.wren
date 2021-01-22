class DBManager {
	// Add a hook when loading a given asset into the game. This returns a DBAssetHook object.
	// Internally the name and extension of all assets is stored as idstrings. The name and ext
	// values may be a regular string (and will automatically be converted to an idstring) or may
	// start with an at symbol ('@') followed by exactly 16 characters in the range 0-9 and a-z.
	foreign static register_asset_hook(name, ext)
}

foreign class DBAssetHook {
	// Boolean, if true then the asset will only be loaded if it
	//  fails to load via the normal method. Default false.
	foreign fallback
	foreign fallback=(val)

	// String, returns how the asset represented by this hook will
	//  be loaded. Can be any of the following:
	// * disabled - asset will be loaded as normal, default after register_asset_hook is called
	// * plain_file - asset will be loaded from a file, same as DB:add
	// * direct_bundle - asset will be loaded directly from a bundle file, which
	//     does not have to be loaded via the package system
	// * wren_loader - asset will be loaded by calling a method of a Wren class
	// For each of these methods there is a getter and a setter. The mode is that of the
	//  last setter called, and the getters return null (or false for 'enabled') if they're
	//  not selected. Getters must not be called with null, doing so will crash the game.
	foreign mode

	// Get whether or not this hook actually does anything - this can be used to disable a hook
	//  without removing it (though this should not be done for performance reasons - removing hooks
	//  is extremely fast) and by default a hook will be disabled until you set an action on it.
	foreign enabled // Returns boolean
	foreign disable() // Returns null

	// Load a file from the filesystem, in a similar vein to DB:add.
	// This only loads the parts of the file into RAM as they're needed, and should generally
	// perform very nicely - this is all the same for direct_bundle.
	foreign plain_file // Returns string or null
	foreign plain_file=(path) // Returns null

	// Load a file directly from the bundle system. Using this a lot will perform very badly on
	//  a mechanical drive, which is most likely why Overkill requires you to load packages in the
	//  first place. This is a very notable method, since it lets you (so long as you're fine with
	//  an SSD being a requirement to use your mod) use files with complete disregard for what
	//  package it's in, while only using the memory of those assetes you actually load.
	// Note that this ignores mod_overrides, DB:add and any other DBAsssetHooks.
	// getter: The value will be null (if this mode is not selected) or in the format of @name-hash.@ext-hash such
	//  that the return value may be used as a DBManager.register_asset_hook-compatible asset id, after
	//  being split on the full stop.
	// setter: The arguments will be subject to the same automatic hashing as DBManager.register_asset_hook - note
	//  that unlike all the other setters, this isn't a real Wren setter - since there's two arguments it's a
	//  regular function.
	foreign direct_bundle // Returns string or null
	foreign set_direct_bundle(name, ext) // Returns null

	// Load this file by calling a method on a Wren object.
	// This object must have a method with the signature 'load_file(_, _)' where the first argument
	//  is the name of the asset that needs to be loaded, and it's second argument is it's
	//  extension (both in the same format as returned by IO.idstring_hash).
	// It must return a DBForeignFile object representing the asset to be passed back to the game.
	// Note: this function may be called many, many times for the same file! If you're doing something
	//  slow (such as building the contents of a file to pass into DBForeignFile.from_string) you may
	//  want to consider caching it.
	// Note: Since Wren is not thread-safe, we have to lock it while running any Wren code. Thus PD2
	//  cannot load multiple files using this method in parallel. This generally shouldn't be a major
	//  issue, but it's one of many reasons you should use plain_file or direct_bundle over this
	//  if possible.
	foreign wren_loader // Returns a user wren object or null
	foreign wren_loader=(val) // Returns null
}

// A description of a file for use by the wren_loader.
// Note that this instance doesn't load the contents of a file - it only stores references to it, such
//  as the filename and offset/length.
//  (that's excluding from_string of course - that has to store the string you pass into it)
foreign class DBForeignFile {
	///// Constructor methods:

	// Use the contents of a file, relative to the game's folder (you'll have to include the mods/yourmod
	//  prefix yourself).
	foreign static of_file(filename)

	// Wrap an existing asset in PD2's database. The name and ext arguments follow the same automatic
	//  hashing rules as DBManager.register_asset_hook
	foreign static of_asset(name, ext)

	// Accepts a string and builds a foreign file using that (encoded with UTF-8) as the contents.
	// Note: DO NOT TRY THIS WITH BINARY BLOBS. Unlike Lua, wren cares a lot about the contents of
	//  it's strings and fiddling around to fit binary data in will lead to various crashes, both
	//  those intentionally triggered by Wren and possibly also others.
	// If you need to inject a file, this is a very slow way to do it - of_file is much faster way
	//  that doesn't require storing the contents of the file in memory while not in use.
	// This is probably the most powerful method SBLT supports for loading assets, and has many
	//  fantastic uses (such as potentally generating unit files on-the-fly) but also some potential
	//  performance issues.
	foreign static from_string(str)
}
