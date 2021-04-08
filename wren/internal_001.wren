// Internal functions that the basemod and DLL can use to communicate

class Internal {
    foreign static tweaker_enabled=(value) // Disable the tweaker if the basemod is using the DB hook system instead

    // Show a UI to warn that a mod failed to load
    // This is intentionally restrictive to avoid abuse to show random popups, which
    // maybe we should add in it's own API later.
    foreign static warn_bad_mod(name, err)
}
