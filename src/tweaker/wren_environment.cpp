//
// Created by znix on 08/04/2021.
//

#include "wren_environment.h"

#include <assert.h>
#include <cstdio>
#include <string.h>
#include <string>

// Hack to get the stack trace
extern "C"
{
#include "../../lib/wren/src/vm/wren_vm.h"
}

void get_mod_directory(WrenVM* vm)
{
	// Poke around in Wren's internals to do this - not ideal but hey it works (for now...)
	ObjFiber* fibre = vm->fiber;

	ObjFn* caller = fibre->frames[fibre->numFrames - 1].closure->fn;
	std::string callerModule = caller->module->name->value;

	// Chop it down to the mod name
	std::string mod = callerModule;
	mod.erase(mod.find_first_of("/"));

	// Convert it to the mod directory
	// If we let people change the mod name distinct from it's directory, we'll have to
	// change this.
	std::string dir = "mods/" + mod;

	wrenSetSlotString(vm, 0, dir.c_str());
}

WrenForeignMethodFn pd2hook::tweaker::wren_env::bind_wren_env_method(WrenVM* vm, const char* module,
                                                                     const char* className, bool isStatic,
                                                                     const char* signature)
{
	if (strcmp(module, "base/native/Environment_001") == 0)
	{
		if (strcmp(className, "Environment") == 0)
		{
			if (isStatic && strcmp(signature, "mod_directory") == 0)
			{
				return &get_mod_directory;
			}
		}
	}

	return nullptr;
}
