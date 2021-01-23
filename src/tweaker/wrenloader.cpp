#include "wrenloader.h"

#include <fstream>
#include <vector>

#include "db_hooks.h"
#include "global.h"
#include "plugins/plugins.h"
#include "util/util.h"
#include "wrenxml.h"
#include "xmltweaker_internal.h"

#include <wren.hpp>

#include "wren_generated_src.h"

using namespace pd2hook;
using namespace pd2hook::tweaker;
using namespace std;

static void err([[maybe_unused]] WrenVM* vm, [[maybe_unused]] WrenErrorType type, const char* module, int line,
                const char* message)
{
	if (module == nullptr)
		module = "<unknown>";
	PD2HOOK_LOG_LOG(string("[WREN ERR] ") + string(module) + ":" + to_string(line) + " ] " + message);
}

static void log(WrenVM* vm)
{
	const char* text = wrenGetSlotString(vm, 1);
	PD2HOOK_LOG_LOG(string("[WREN] ") + text);
}

static string file_to_string(ifstream& in)
{
	return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
}

void io_listDirectory(WrenVM* vm)
{
	string filename = wrenGetSlotString(vm, 1);
	bool dir = wrenGetSlotBool(vm, 2);
	vector<string> files = Util::GetDirectoryContents(filename, dir);

	wrenSetSlotNewList(vm, 0);

	for (string const& file : files)
	{
		if (file == "." || file == "..")
			continue;

		wrenSetSlotString(vm, 1, file.c_str());
		wrenInsertInList(vm, 0, -1, 1);
	}
}

void io_info(WrenVM* vm)
{
	const char* path = wrenGetSlotString(vm, 1);

	Util::FileType type = Util::GetFileType(path);

	if (type == Util::FileType_None)
	{
		wrenSetSlotString(vm, 0, "none");
	}
	else if (type == Util::FileType_Directory)
	{
		wrenSetSlotString(vm, 0, "dir");
	}
	else
	{
		wrenSetSlotString(vm, 0, "file");
	}
}

void io_read(WrenVM* vm)
{
	string file = wrenGetSlotString(vm, 1);

	ifstream handle(file);
	if (!handle.good())
	{
		PD2HOOK_LOG_ERROR("Wren IO.read: Could not load file " + file);
		exit(1);
	}

	string contents = file_to_string(handle);
	wrenSetSlotString(vm, 0, contents.c_str());
}

void io_idstring_hash(WrenVM* vm)
{
	blt::idstring hash = idstring_hash(wrenGetSlotString(vm, 1));

	char hex[17]; // 16-chars long +1 for the null
	snprintf(hex, sizeof(hex), "%016llx", hash);
	wrenSetSlotString(vm, 0, hex);
}

static void io_load_plugin(WrenVM* vm)
{
	const char* plugin_filename = wrenGetSlotString(vm, 1);
	try
	{
		blt::plugins::LoadPlugin(plugin_filename);
	}
	catch (const string& err)
	{
		string msg = string("LoadPlugin: ") + string(plugin_filename) + string(" : ") + err;
		wrenSetSlotString(vm, 0, msg.c_str());
		wrenAbortFiber(vm, 0);
	}
}

static void internal_set_tweaker_enabled(WrenVM* vm)
{
	pd2hook::tweaker::tweaker_enabled = wrenGetSlotBool(vm, 1);
}

static WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module, const char* class_name)
{
	WrenForeignClassMethods methods = wrenxml::get_XML_class_def(vm, module, class_name);
	if (methods.allocate || methods.finalize)
		return methods;

	methods = dbhook::bind_dbhook_class(vm, module, class_name);

	return methods;
}

static WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module, const char* className, bool isStatic,
                                             const char* signature)
{
	WrenForeignMethodFn wxml_method = wrenxml::bind_wxml_method(vm, module, className, isStatic, signature);
	if (wxml_method)
		return wxml_method;

	WrenForeignMethodFn dbhook_method = dbhook::bind_dbhook_method(vm, module, className, isStatic, signature);
	if (dbhook_method)
		return dbhook_method;

	if (strcmp(module, "base/native") == 0)
	{
		if (strcmp(className, "Logger") == 0)
		{
			if (isStatic && strcmp(signature, "log(_)") == 0)
			{
				return &log; // C function for Math.add(_,_).
			}
			// Other foreign methods on Math...
		}
		else if (strcmp(className, "IO") == 0)
		{
			if (isStatic && strcmp(signature, "listDirectory(_,_)") == 0)
			{
				return &io_listDirectory;
			}
			if (isStatic && strcmp(signature, "info(_)") == 0)
			{
				return &io_info;
			}
			if (isStatic && strcmp(signature, "read(_)") == 0)
			{
				return &io_read;
			}
			if (isStatic && strcmp(signature, "idstring_hash(_)") == 0)
			{
				return &io_idstring_hash;
			}
			if (isStatic && strcmp(signature, "load_plugin(_)") == 0)
			{
				return &io_load_plugin;
			}
		}
		// Other classes in main...
	}
	else if (strcmp(module, "base/native/internal_001") == 0)
	{
		if (strcmp(className, "Internal") == 0)
		{
			if (isStatic && strcmp(signature, "tweaker_enabled=(_)") == 0)
			{
				return &internal_set_tweaker_enabled;
			}
		}
	}
	// Other modules...

	return nullptr;
}

static WrenLoadModuleResult getModulePath([[maybe_unused]] WrenVM* vm, const char* name_c)
{
	// First see if this is a module that's embedded within SuperBLT
	const char* builtin_string = nullptr;
	lookup_builtin_wren_src(name_c, &builtin_string);
	if (builtin_string)
	{
		WrenLoadModuleResult result{};
		result.source = builtin_string;
		return result;
	}

	// Otherwise it's a normal wren file, load it from the appropriate mod
	string name = name_c;
	string mod = name.substr(0, name.find_first_of('/'));
	string file = name.substr(name.find_first_of('/') + 1);

	ifstream handle("mods/" + mod + "/wren/" + file + ".wren");
	if (!handle.good())
	{
		WrenLoadModuleResult result{};
		return result;
	}

	string str = file_to_string(handle);

	// Perhaps unwisely I used 'continue' as a variable name in xml_loader.wren in the basemod, which
	// is now a keyword. To avoid crashes if someone updates their DLL before updating their basemod, check
	// for that and hack around it as necessary.
	if (name == "base/private/xml_loader" && str.find("var continue = dive_tweak_elem") != std::string::npos)
	{
		// Oh by the way, thanks C++ for not having a string find-replace function (unless I can't find it)
		size_t pos = 0;
		while ((pos = str.find("continue", pos)) != std::string::npos)
		{
			str.replace(pos, 8, "cont");
		}
		PD2HOOK_LOG_WARN("Patching around an old use of the variable name 'continue'. Please update your basemod.");
	}

	size_t length = str.length() + 1;
	char* output = (char*)malloc(length); // +1 for the null
	portable_strncpy(output, str.c_str(), length);

	WrenLoadModuleResult result{};
	result.source = output;
	result.onComplete = [](WrenVM*, const char* module, WrenLoadModuleResult result) { free((void*)result.source); };
	return result;
}

std::lock_guard<std::recursive_mutex> pd2hook::wren::lock_wren_vm()
{
	static std::recursive_mutex vm_mutex;
	return std::lock_guard<std::recursive_mutex>(vm_mutex);
}

WrenVM* pd2hook::wren::get_wren_vm()
{
	auto lock = lock_wren_vm();

	static bool available = true;
	static WrenVM* vm = nullptr;

	if (vm == nullptr)
	{
		if (available)
		{
			// If the main file doesn't exist, do nothing
			Util::FileType ftyp = Util::GetFileType("mods/base/wren/base.wren");
			if (ftyp == Util::FileType_None)
				available = false;
		}

		if (!available)
			return nullptr;

		WrenConfiguration config;
		wrenInitConfiguration(&config);
		config.errorFn = &err;
		config.bindForeignMethodFn = &bindForeignMethod;
		config.bindForeignClassFn = &bindForeignClass;
		config.loadModuleFn = &getModulePath;
		vm = wrenNewVM(&config);

		WrenInterpretResult result = wrenInterpret(vm, "__root", R"!( import "base/base" )!");
		if (result == WREN_RESULT_COMPILE_ERROR || result == WREN_RESULT_RUNTIME_ERROR)
		{
			PD2HOOK_LOG_ERROR("Wren init failed: compile or runtime error!");

#ifdef _WIN32
			MessageBox(nullptr, "Failed to initialise the Wren system - see the log for details", "Wren Error", MB_OK);
			ExitProcess(1);
#else
			abort();
#endif
		}
	}

	return vm;
}

const char* tweaker::transform_file(const char* text)
{
	auto lock = pd2hook::wren::lock_wren_vm();
	WrenVM* vm = pd2hook::wren::get_wren_vm();

	wrenEnsureSlots(vm, 4);

	wrenGetVariable(vm, "base/base", "BaseTweaker", 0);
	WrenHandle* tweakerClass = wrenGetSlotHandle(vm, 0);
	WrenHandle* sig = wrenMakeCallHandle(vm, "tweak(_,_,_)");

	char hex[17]; // 16-chars long +1 for the null

	wrenSetSlotHandle(vm, 0, tweakerClass);

	snprintf(hex, sizeof(hex), "%016llx", *blt::platform::last_loaded_name);
	wrenSetSlotString(vm, 1, hex);

	snprintf(hex, sizeof(hex), "%016llx", *blt::platform::last_loaded_ext);
	wrenSetSlotString(vm, 2, hex);

	wrenSetSlotString(vm, 3, text);

	// TODO give a reasonable amount of information on what happened.
	WrenInterpretResult result2 = wrenCall(vm, sig);
	if (result2 == WREN_RESULT_COMPILE_ERROR)
	{
		PD2HOOK_LOG_ERROR("Wren tweak file failed: compile error!");
		return text;
	}
	else if (result2 == WREN_RESULT_RUNTIME_ERROR)
	{
		PD2HOOK_LOG_ERROR("Wren tweak file failed: runtime error!");
		return text;
	}

	wrenReleaseHandle(vm, tweakerClass);
	wrenReleaseHandle(vm, sig);

	const char* new_text = wrenGetSlotString(vm, 0);

	return new_text;
}
