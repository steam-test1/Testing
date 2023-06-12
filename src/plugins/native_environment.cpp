#include "native_environment.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <util/util.h>

bool blt::plugins::environment::isVr()
{
	bool is_vr = false;

// Same logic as wren, maybe make all of these shared in some file?
#ifdef _WIN32
	TCHAR processPath[MAX_PATH + 1];
	GetModuleFileName(NULL, processPath, MAX_PATH + 1);
	std::string processPathString = processPath;
	is_vr = processPathString.rfind("_vr.exe") == processPathString.length() - 7;
#endif

	return is_vr;
}

// can't seem to get this working
const char* blt::plugins::environment::getModDirectory(const char* nativeModulePath)
{
	return "";
}

unsigned long long blt::plugins::environment::hashString(const char* str)
{
	return blt::idstring_hash(str);
}