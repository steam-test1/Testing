#include "blt/elf_utils.hh"

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
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_call"));

	realCall(L, libname, l, nup);
}

typedef int (*lua_CFunction) (void *L);
extern "C" void lua_pushcclosure(void* L, lua_CFunction fn, int n)
{
	using FuncType = void (*)(void*, lua_CFunction, int);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_call"));

	realCall(L, fn, n);
}
