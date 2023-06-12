#include "InitState.h"
#include "platform.h"
#include "plugins/plugins.h"
#include "util/util.h"

#include "plugins/native_db_hooks.h"
#include "plugins/native_environment.h"

#include <functional>

using namespace blt::plugins;

// Don't use these funcdefs when porting to GNU+Linux, as it doesn't need
// any kind of getter function because the PD2 binary isn't stripped
typedef void* (*lua_access_func_t)(const char*);
typedef void (*init_func_t)(lua_access_func_t get_lua_func_by_name);

static void pd2_log(const char* message, int level, const char* file, int line)
{
	using LT = pd2hook::Logging::LogType;

	char buffer[256];
	sprintf_s(buffer, sizeof(buffer), "ExtModDLL %s", file);

	switch ((LT)level)
	{
	case LT::LOGGING_FUNC:
	case LT::LOGGING_LOG:
	case LT::LOGGING_LUA:
		PD2HOOK_LOG_LEVEL(message, (LT)level, buffer, line, FOREGROUND_RED, FOREGROUND_BLUE, FOREGROUND_INTENSITY);
		break;
	case LT::LOGGING_WARN:
		PD2HOOK_LOG_LEVEL(message, (LT)level, buffer, line, FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_INTENSITY);
		break;
	case LT::LOGGING_ERROR:
		PD2HOOK_LOG_LEVEL(message, (LT)level, buffer, line, FOREGROUND_RED, FOREGROUND_INTENSITY);
		break;
	}
}

static bool is_active_state(lua_State* L)
{
	return pd2hook::check_active_state(L);
}

enum class HookMode
{
	REPLACER = 0,
	PLAIN_FILE,
	DIRECT_BUNDLE,
};

void pd2_db_hook_asset_file(const char* name, const char* ext, std::function<void(std::vector<uint8_t>*)> replacer,
                            const char* plain_file, const char* direct_bundle_name, const char* direct_bundle_ext,
                            HookMode mode)
{
	blt::plugins::dbhook::DBTargetFile* target = nullptr;

	blt::plugins::dbhook::registerAssetHook(name, ext, false, &target);

	if (mode == HookMode::REPLACER)
	{
		target->setReplacer(replacer);
	}
	else if (mode == HookMode::PLAIN_FILE)
	{
		target->setPlainFile(plain_file);
	}
	else if (mode == HookMode::DIRECT_BUNDLE)
	{
		target->setDirectBundle(direct_bundle_name, direct_bundle_ext);
	}
}

std::vector<uint8_t> pd2_db_find_file(const char* name, const char* ext)
{
    return blt::plugins::dbhook::findFile(name, ext);
}

bool pd2_db_file_exists(const char* name, const char* ext)
{
    return blt::plugins::dbhook::fileExists(name, ext);
}

bool pd2_is_vr()
{
	return blt::plugins::environment::isVr();
}

const char* pd2_get_mod_directory_internal(const char* nativeModulePath)
{
	return blt::plugins::environment::getModDirectory(nativeModulePath);
}

unsigned long long pd2_create_hash(const char* str)
{
	return blt::plugins::environment::hashString(str);
}

static void* get_func(const char* name)
{
	std::string str = name;

	if (str == "pd2_log")
	{
		return &pd2_log;
	}
	else if (str == "is_active_state")
	{
		return &is_active_state;
	}
	else if (str == "luaL_checkstack")
	{
		return &luaL_checkstack;
	}
	else if (str == "lua_rawequal")
	{
		return &lua_rawequal;
	}
	// wren and lua function parity
	else if (str == "pd2_db_hook_asset_file")
	{
		return &pd2_db_hook_asset_file;
	}
	else if (str == "pd2_db_find_file")
	{
		return &pd2_db_find_file;
	}
	else if (str == "pd2_is_vr")
	{
		return &pd2_is_vr;
	}
	else if (str == "pd2_get_mod_directory_internal")
	{
		return &pd2_get_mod_directory_internal;
	}
	else if (str == "pd2_create_hash")
	{
		return &pd2_create_hash;
	}
    else if (str == "pd2_db_file_exists")
    {
        return &pd2_db_file_exists;
    }

	return blt::platform::win32::get_lua_func(name);
}

class WindowsPlugin : public Plugin
{
  public:
	WindowsPlugin(std::string file);

  protected:
	virtual void* ResolveSymbol(std::string name) const;

  private:
	HMODULE module;
};

WindowsPlugin::WindowsPlugin(std::string file) : Plugin(file)
{
	module = LoadLibraryA(file.c_str());

	if (!module)
		throw std::string("Failed to load module: ERR") + std::to_string(GetLastError());

	Init();

	// Start loading everything
	init_func_t init = (init_func_t)GetProcAddress(module, "SuperBLT_Plugin_Setup");
	if (!init)
		throw "Invalid module - missing initfunc!";

	init(get_func);
}

void* WindowsPlugin::ResolveSymbol(std::string name) const
{
	return GetProcAddress(module, name.c_str());
}

Plugin* blt::plugins::CreateNativePlugin(std::string file)
{
	return new WindowsPlugin(file);
}
