// TODO split this out or something idk
#include "blt/elf_utils.hh"

extern "C" {
#include <dlfcn.h>
#include <dirent.h>
}

#include <iostream>
#include <list>
#include <string>
#include <subhook.h>

#include <lua.h>
#include <platform.h>

#include <dsl/LuaInterface.hh>
#include <dsl/FileSystem.hh>

#include <blt/error.hh>
#include <blt/log.hh>
#include <blt/assets.hh>
#include <blt/lapi_compat.hh>
#include <blt/png_parser.hh>

#include <InitState.h>
#include <lua_functions.h>
#include <util/util.h>
#include <dbutil/DB.h>

#include <tweaker/xmltweaker.h> // TODO move into the standard interface API

/**
 * Shorthand to reinstall a hook when a function exits, like a trampoline
 */
#define hook_remove(hookName) subhook::ScopedHookRemove _sh_remove_raii(&hookName)

namespace blt
{

	using std::cerr;
	using std::cout;
	using std::string;
	using subhook::Hook;
	using subhook::HookFlag64BitOffset;

	void* (*dsl_lua_newstate) (dsl::LuaInterface* /* this */, bool, bool, dsl::LuaInterface::Allocation);
	void* (*do_game_update)   (void* /* this */);
	void* (*node_from_xml)   (void* /* this */, const char *, const uint64_t, void*);

	/*
	 * Detour Impl
	 */

	Hook     gameUpdateDetour;
	Hook     luaNewStateDetour;
	Hook     luaCallDetour;
	Hook     luaCloseDetour;
	Hook     nodeFromXMLDetour;
	bool     forcepcalls = false;

	void*
	dt_Application_update(void* parentThis)
	{
		hook_remove(gameUpdateDetour);

		lua_state* L = **(lua_state* **)( (char*)parentThis + 696 );
		blt::lua_functions::update(L);

		return do_game_update(parentThis);
	}

	void
	dt_lua_call(lua_state* state, int argCount, int resultCount)
	{
		hook_remove(luaCallDetour);

		if(forcepcalls)
		{
			return blt::lua_functions::perform_lua_pcall(state, argCount, resultCount);
		}
		else
		{
			const int target = 1; // Push our handler to the start of the stack

			// Get the value onto the stack, as pcall can't accept indexes
			error::push_callback(state);
			lua_insert(state, target);

			// Run the function, and any errors are handled for us.
			lua_pcall(state, argCount, resultCount, target);

			// Done with our error handler
			lua_remove(state, target);
		}
	}

	void*
	dt_dsl_lua_newstate(dsl::LuaInterface* _this, bool b1, bool b2, dsl::LuaInterface::Allocation allocator)
	{
		hook_remove(luaNewStateDetour);

		void* returnVal = _this->newstate(b1, b2, allocator);

		lua_state* state = _this->state;

		if (!state)
		{
			return returnVal;
		}

		// First, add our own compatibility stuff in
		compat::add_members(state);

		blt::lua_functions::initiate_lua(state);

		// Add our custom Lua members - ie, the blt.create_entry function
		asset_add_lua_members(state);

		// If we haven't already set up the PNG handler, do so now
		// The reason we don't do it at startup is that PD2's global constructor would overwrite it.
		static bool hasInstalledPngHandler = false;
		if (!hasInstalledPngHandler)
		{
			install_png_parser();
		}

		return returnVal;
	}

	void
	dt_lua_close(lua_state* state)
	{
		blt::lua_functions::close(state);
		hook_remove(luaCloseDetour);
		lua_close(state);
	}

	void dt_node_from_xml(void *this_, const char *string, const uint64_t length, void *something)
	{
		hook_remove(nodeFromXMLDetour);

		// TODO move into the standard interface API
		char *newstr = (char*) pd2hook::tweaker::tweak_pd2_xml((char*) string, length);
		node_from_xml(this_, newstr, length, something);
		pd2hook::tweaker::free_tweaked_pd2_xml(newstr);
	}

	namespace platform
	{
		void InitPlatform() {}
		void ClosePlatform() {}

		idstring *last_loaded_ext = NULL, *last_loaded_name = NULL;

		namespace lua
		{
			bool GetForcePCalls()
			{
				return forcepcalls;
			}

			void SetForcePCalls(bool state)
			{
				forcepcalls = state;
			}
		}
	}

	void
	blt_init_hooks()
	{
		// Load this first, so if something goes wrong we don't have to wait for the game to start
		blt::db::DieselDB::Instance();

		// First thing to do, install error handlers so if we crash we can generate a nice stacktrace
		log::log("Installing SuperBLT error handlers", log::LOG_INFO);
        error::set_global_handlers();

#define setcall(symbol,ptr) *(void**)(&ptr) = blt::elf_utils::find_sym(#symbol); \
		if (ptr == nullptr) { PD2HOOK_LOG_LOG(#ptr " was null"); abort(); }

		log::log("finding lua functions", log::LOG_INFO);

		/*
		 * XXX Still using the ld to get member function bodies from memory, since pedantic compilers refuse to allow
		 * XXX non-static instanceless member function references
		 * XXX (e.g. clang won't allow a straight pointer to _ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE via `&dsl::LuaInterface::newstate`)
		 */

		{
			// _ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE = dsl::LuaInterface::newstate(...)
			setcall(_ZN3dsl12LuaInterface8newstateEbbNS0_10AllocationE, dsl_lua_newstate);

			// _ZN11Application6updateEv = Application::update()
			setcall(_ZN11Application6updateEv, do_game_update);

			// _ZN3dsl8xmlparse13node_from_xmlERNS_4NodeEPKcRKm = dsl::xmlparse::node_from_xml(...)
			setcall(_ZN3dsl8xmlparse13node_from_xmlERNS_4NodeEPKcRKm, node_from_xml);
		}

		// Grab the IDstring things
		platform::last_loaded_name = (idstring*) blt::elf_utils::find_sym("_ZN3dsl14LoadingTracker8_db_nameE");
		platform::last_loaded_ext  = (idstring*) blt::elf_utils::find_sym("_ZN3dsl14LoadingTracker8_db_typeE");

		PD2HOOK_LOG_LOG("installing hooks");

		/*
		 * Intercept Init
		 */

		{
			gameUpdateDetour.Install    ((void*) do_game_update,                (void*) dt_Application_update, HookFlag64BitOffset);
			luaNewStateDetour.Install   ((void*) dsl_lua_newstate,              (void*) dt_dsl_lua_newstate, HookFlag64BitOffset);
			luaCloseDetour.Install      ((void*) &lua_close,                    (void*) dt_lua_close, HookFlag64BitOffset);
			luaCallDetour.Install       ((void*) &lua_call,                     (void*) dt_lua_call, HookFlag64BitOffset);

			nodeFromXMLDetour.Install   ((void*) node_from_xml,                 (void*) dt_node_from_xml, HookFlag64BitOffset);
		}

		init_asset_hook();
		init_png_parser();

		pd2hook::InitiateStates(); // TODO move this into the blt namespace
#undef setcall
	}

}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
