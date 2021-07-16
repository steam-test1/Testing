#include "blt/elf_utils.hh"
#include <cstdio>

typedef int (*lua_CFunction) (void *L);

extern "C" void lua_close(void* L)
{
	using FuncType = void (*)(void*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_close"));

	realCall(L);
}

extern "C" void lua_call(void* L, int nargs, int nresults)
{
	using FuncType = void (*)(void*, int, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_call"));

	realCall(L, nargs, nresults);
}

extern "C" void luaL_openlib(void* L, const char* libname, const void* l, int nup)
{
	using FuncType = void (*)(void*, const char*, const void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_openlib"));

	realCall(L, libname, l, nup);
}

extern "C" void lua_pushcclosure(void* L, lua_CFunction fn, int n)
{
	using FuncType = void (*)(void*, lua_CFunction, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushcclosure"));

	realCall(L, fn, n);
}

extern "C" void lua_setfield(void* L, int index, const char* k)
{
	using FuncType = void (*)(void*, int, const char*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setfield"));

	realCall(L, index, k);
}

extern "C" void lua_settop(void* L, int index)
{
	using FuncType = void (*)(void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_settop"));

	realCall(L, index);
}

extern "C" void lua_createtable(void* L, int narr, int nrec)
{
	using FuncType = void (*)(void*, int, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_createtable"));

	realCall(L, narr, nrec);
}

extern "C" int luaL_newmetatable(void* L, const char* tname)
{
	using FuncType = int (*)(void*, const char*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_newmetatable"));

	return realCall(L, tname);
}

extern "C" void lua_pushstring(void* L, const char* s)
{
	using FuncType = void (*)(void*, const char*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushstring"));

	realCall(L, s);
}

extern "C" void lua_pushvalue(void* L, int index)
{
	using FuncType = void (*)(void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushvalue"));

	realCall(L, index);
}

extern "C" void lua_settable(void* L, int index)
{
	using FuncType = void (*)(void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_settable"));

	realCall(L, index);
}

extern "C" void lua_getfield(void* L, int index, const char* k)
{
	using FuncType = void (*)(void*, int, const char*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getfield"));

	realCall(L, index, k);
}

extern "C" int luaL_loadfilex(void* L, const char* filename, const char* mode)
{
	using FuncType = int (*)(void*, const char*, const char*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_loadfilex"));

	return realCall(L, filename, mode);
}

extern "C" int lua_type(void* L, int index)
{
	using FuncType = int (*)(void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_type"));

	return realCall(L, index);
}

extern "C" void lua_insert(void* L, int index)
{
	using FuncType = void (*)(void*, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_insert"));

	realCall(L, index);
}

extern "C" int lua_pcall(void* L, int nargs, int nresults, int errfunc)
{
	using FuncType = int (*)(void*, int, int, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pcall"));

	return realCall(L, nargs, nresults, errfunc);
}

extern "C" const char* lua_tolstring(void* L, int index, size_t* len)
{
	using FuncType = const char* (*)(void*, int, size_t*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tolstring"));

	return realCall(L, index, len);
}

extern "C" int lua_getstack(void* L, int level, void* ar)
{
	using FuncType = int (*)(void*, int, void*);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getstack"));

	return realCall(L, level, ar);
}
