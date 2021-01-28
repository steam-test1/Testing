//
// Created by znix on 28/01/2021.
//

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void load_sblt(void);

__attribute__((constructor)) static void shim_main()
{
	// See if a symbol from PD2 is present, if so we're ready to load the main mod library
	void* dlHandle = dlopen(NULL, RTLD_LAZY);

	if (dlHandle && dlsym(dlHandle, "_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE"))
	{
		load_sblt();
	}

	dlclose(dlHandle);
}

static void load_sblt()
{
	const char* env_name = "SBLT_LOADER";
	const char* path = getenv(env_name);
	if (!path)
	{
		fprintf(stderr, "SBLT development shim - missing %s environment variable\n", env_name);
		exit(1);
	}

	printf("SBLT Development shim: loading now\n");

	void* handle = dlopen(path, RTLD_NOW);
	if (!handle)
	{
		fprintf(stderr, "SBLT shim: failed to load '%s' - check your %s environment variable\n", path, env_name);
		exit(1);
	}

	// At this point, SBLT will have loaded itself
	dlclose(handle);
}
