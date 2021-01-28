# SuperBLT Linux loader shim

SuperBLT for Linux now has a 'shim' - this is small library that
can be added to LD_PRELOAD, which will then load the main SBLT
library only when it detects it's running inside PAYDAY 2.

This is great for debugging: if you try running anything other
than PAYDAY 2 with the main library in LD_PRELOAD it will fail
with dynamic linker errors, as it imports many PD2 symbols. The
shim circumvents this.

To use it, put the `libdebug_shim.so` in your `LD_PRELOAD` variable
instead of `libsuperblt_loader.so`, and set `SBLT_LOADER` to the
full path of `libsuperblt_loader.so`.
