//
// Created by ZNix on 16/08/2020.
//

#include "assets.h"
#include "platform.h"
#include "subhook.h"

#include <stdio.h>

#include <string>
#include <utility>

// A wrapper class to store strings in the format that PAYDAY 2 does
// Since Microsoft might (and it seems they have) change their string class, we
// need this.
class PDString
{
  public:
	explicit PDString(std::string str) : s(std::move(str))
	{
		data = s.c_str();
		len = s.length();
		cap = s.length();
	}

  private:
	// The data layout that mirrors PD2's string, must be 24 bytes
	const char* data;
	uint8_t padding[12]{};
	int len;
	int cap;

	// String that can deal with the actual data ownership
	std::string s;
};
static_assert(sizeof(PDString) == 24 + sizeof(std::string), "PDString is the wrong size!");

// The signature is the same for all try_open methods, so one typedef will work for all of them.
typedef void(__thiscall* try_open_t)(void* this_, void* archive, int a, int b, blt::idstring type, blt::idstring name);

static void hook_load(try_open_t orig, subhook::Hook& hook, void* this_, void* archive, int u1, int u2,
                      blt::idstring type, blt::idstring name);

#define DECLARE_PASSTHROUGH(func)                                                                        \
	static subhook::Hook hook_##func;                                                                    \
	void __fastcall stub_##func(void* this_, int edx, void* archive, int u1, int u2, blt::idstring type, \
	                            blt::idstring name)                                                      \
	{                                                                                                    \
		hook_load((try_open_t)func, hook_##func, this_, archive, u1, u2, type, name);                    \
	}

DECLARE_PASSTHROUGH(try_open_funcptr);
DECLARE_PASSTHROUGH(try_open_property_match_resolver);

static void hook_load(try_open_t orig, subhook::Hook& hook, void* this_, void* archive, int u1, int u2,
                      blt::idstring type, blt::idstring name)
{

	// printf("Load %p: %016llx.%016llx\n", orig, name, type);

	// At this point, if we want to override this archive we can load the archive ourselves and return now

	subhook::ScopedHookRemove scoped_remove(&hook);
	orig(this_, archive, u1, u2, type, name);
}

void blt::win32::InitAssets()
{
	// load_test_base = reinterpret_cast<load_test_f>(0x00437510);
	// load_test_base = reinterpret_cast<load_test_f>(0x0067bc90);
	// load_test_base = reinterpret_cast<load_test_f>(0x0067c0f0);
	// load_test_base = reinterpret_cast<load_test_f>(0x006fbaa0);
	// load_test_base = reinterpret_cast<load_test_f>(0x00756080);
	// load_test_hook.Install(load_test_base, load_test);

#define SETUP_PASSTHROUGH(func) hook_##func.Install(func, stub_##func)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-cast"
	SETUP_PASSTHROUGH(try_open_funcptr);
	SETUP_PASSTHROUGH(try_open_property_match_resolver);
#pragma clang diagnostic pop
}
