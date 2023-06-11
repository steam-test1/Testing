#include "plugins/plugins.h"
#include "util/util.h"
#include "InitState.h"
#include "platform.h"

#include "plugins/native_db_hooks.h"

#include <functional>

using namespace blt::plugins;

// Don't use these funcdefs when porting to GNU+Linux, as it doesn't need
// any kind of getter function because the PD2 binary isn't stripped
typedef void*(*lua_access_func_t)(const char*);
typedef void(*init_func_t)(lua_access_func_t get_lua_func_by_name);

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

static bool is_active_state(lua_State *L)
{
	return pd2hook::check_active_state(L);
}

void pd2_db_hook_asset_file(const char* name, const char* ext, std::function<void(std::vector<uint8_t>*)> replacer) {
    blt::idstring nameids = blt::idstring(name);
    blt::idstring extids = blt::idstring(ext);

    blt::plugins::dbhook::DBTargetFile* file = nullptr;

    blt::plugins::dbhook::registerAssetHook(name, ext, false, &file);

    file->setReplacer(replacer);
}

static void * get_func(const char* name)
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
	} else if(str == "pd2_db_hook_asset_file") {
        return &pd2_db_hook_asset_file;
    }

	return blt::platform::win32::get_lua_func(name);
}

class WindowsPlugin : public Plugin {
public:
	WindowsPlugin(std::string file);
protected:
	virtual void *ResolveSymbol(std::string name) const;
private:
	HMODULE module;
};

WindowsPlugin::WindowsPlugin(std::string file) : Plugin(file)
{
	module = LoadLibraryA(file.c_str());

	if (!module) throw std::string("Failed to load module: ERR") + std::to_string(GetLastError());

	Init();

	// Start loading everything
	init_func_t init = (init_func_t)GetProcAddress(module, "SuperBLT_Plugin_Setup");
	if (!init) throw "Invalid module - missing initfunc!";

	init(get_func);
}

void *WindowsPlugin::ResolveSymbol(std::string name) const
{
	return GetProcAddress(module, name.c_str());
}

Plugin *blt::plugins::CreateNativePlugin(std::string file)
{
	return new WindowsPlugin(file);
}
