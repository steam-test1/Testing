/**
 * (C) 2016- Roman Hargrave
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

extern "C" {
#include <dlfcn.h>
}
#include <iostream>
#include <fstream>
#include "blt/hook.hh"
#include "blt/elf_utils.hh"

using std::cerr;

/*
 * Test for a GCC-compatible compiler, because we need
 * the compiler to support the section attribute
 */

#if (!defined(__GNUC__))
#   warn GCC is required for this program to work. ignore this if your compiler supports the constructor attribute
#endif

/*
 * This is run (several) times during pd2 startup.
 * This implements a check to only run once.
 * It will try to use subhook to detour necessary functions
 * and get a dl pointer to the process image
 * we (should) be running in the same process memory as payday2_release, but
 * the kernel VM does a good job of keeping your hands out of another library's shit,
 * unlike windows. This is problematic here since we need to get a linker handle
 * to the payday2_release process image, but can't necessarily do that.
 */
static void
blt_main ()
{
	/*
	 * So in theory, the lines below should capture this already but on Linux, Steam first launches its own launcher
	 * which then launches PD2. In theory this should be all fine and dandy, in practice, however, this own launcher
	 * does not have a .symtab and .strtab which makes `blt::elf_utils::init();` fail. So this is an additional layer
	 * of protection (now that I think of it, the hack below could theoretically be removed. But who knows, maybe this
	 * could _somehow_ still happen (new PD2 version or something?) so I'll let it be for now.
	 */
	std::string sp;
	std::ifstream("/proc/self/comm") >> sp;

	if (sp != "payday2_release") {
		return;
	}

	blt::elf_utils::init();

	/*
	 * Hack: test for presence of a known unique function amongst the libraries loaded by payday
	 * so that we don't try to init on some other image or some shit.
	 *
	 * lua_call will do just fine in this case, as it's only present in payday
	 */
	if (blt::elf_utils::find_sym("_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE"))
	{
		blt::blt_init_hooks();
	}
	else
	{
		cerr << "_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE wasn't found, won't load!\n";
	}
}

/*
 * Put a function pointer to the main function in a special section.
 *
 * This is, by means of our custom link script, loaded last to ensure the static variables
 * from all other parts of this project (including static libraries) are loaded first.
 *
 * I'd prefer not to use a custom link script, but I can't find any other way to get the main
 * function run after the bulk of BLT (in it's own static library now, thanks to the new
 * build system).
 */
void (*init_func)(void) __attribute__(( section(".init_superblt") )) = blt_main;

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
