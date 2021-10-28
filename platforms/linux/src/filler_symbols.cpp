#include "blt/elf_utils.hh"
#include <cstdio>

using lua_State = void;
using lua_Debug = void;
using luaL_Reg = void;
using lua_Number = double;
using lua_Integer = std::ptrdiff_t;

typedef int (*lua_CFunction) (void *L);
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);
typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

/*[[[cog
import re


LUA_FUNCS = """
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);
LUA_API void       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);

LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_remove) (lua_State *L, int idx);
LUA_API void  (lua_insert) (lua_State *L, int idx);
LUA_API void  (lua_replace) (lua_State *L, int idx);
LUA_API int   (lua_checkstack) (lua_State *L, int sz);

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API int            (lua_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (lua_objlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void           *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);

LUA_API void  (lua_pushnil) (lua_State *L);
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt, va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);

LUA_API void  (lua_gettable) (lua_State *L, int idx);
LUA_API void  (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawget) (lua_State *L, int idx);
LUA_API void  (lua_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API void  (lua_getfenv) (lua_State *L, int idx);

LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setfenv) (lua_State *L, int idx);

LUA_API void  (lua_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (lua_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int   (lua_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt, const char *chunkname);

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data);

LUA_API int  (lua_yield) (lua_State *L, int nresults);
LUA_API int  (lua_resume) (lua_State *L, int narg);
LUA_API int  (lua_status) (lua_State *L);

LUA_API int (lua_gc) (lua_State *L, int what, int data);

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);

LUA_API void lua_setlevel       (lua_State *from, lua_State *to);

LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n);

LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook lua_gethook (lua_State *L);
LUA_API int lua_gethookmask (lua_State *L);
LUA_API int lua_gethookcount (lua_State *L);

LUALIB_API void (luaL_openlib) (lua_State *L, const char *libname, const luaL_Reg *l, int nup);
LUALIB_API void (luaL_register) (lua_State *L, const char *libname, const luaL_Reg *l);
LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e);
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e);
LUALIB_API int (luaL_typerror) (lua_State *L, int narg, const char *tname);
LUALIB_API int (luaL_argerror) (lua_State *L, int numarg, const char *extramsg);
LUALIB_API const char *(luaL_checklstring) (lua_State *L, int numArg, size_t *l);
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int numArg, const char *def, size_t *l);
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int numArg);
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int nArg, lua_Number def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int numArg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int nArg, lua_Integer def);

LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg);
LUALIB_API void (luaL_checktype) (lua_State *L, int narg, int t);
LUALIB_API void (luaL_checkany) (lua_State *L, int narg);

LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname);
LUALIB_API void *(luaL_checkudata) (lua_State *L, int ud, const char *tname);

LUALIB_API void (luaL_where) (lua_State *L, int lvl);
LUALIB_API int (luaL_error) (lua_State *L, const char *fmt, ...);

LUALIB_API int (luaL_checkoption) (lua_State *L, int narg, const char *def, const char *const lst[]);

LUALIB_API int (luaL_ref) (lua_State *L, int t);
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref);

LUALIB_API int (luaL_loadfile) (lua_State *L, const char *filename);
LUALIB_API int (luaL_loadfilex) (lua_State *L, const char *filename, const char *mode);
LUALIB_API int (luaL_loadbuffer) (lua_State *L, const char *buff, size_t sz, const char *name);
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s);

LUALIB_API lua_State *(luaL_newstate) (void);


LUALIB_API const char *(luaL_gsub) (lua_State *L, const char *s, const char *p, const char *r);

LUALIB_API const char *(luaL_findtable) (lua_State *L, int idx, const char *fname, int szhint);
"""


func_pattern = re.compile(r"LUA(LIB)?_API ((const )?\w+\s*\*?)\(?(\w+)\)?\s*\((.*)\);")
params_pattern = re.compile(r"((const )?\w+\s*\**(\s*const\s)?)(\w+)(\[\])?,?")

for match in func_pattern.finditer(LUA_FUNCS):
    return_type = match.group(2)
    func_name = match.group(4)
    func_all_args = match.group(5)

    if func_all_args == "void":
        func_all_args = ""

    func_args = list(params_pattern.finditer(func_all_args))

    func_types = ", ".join([arg.group(1) + (arg.group(5) or "") for arg in func_args])
    func_names = ", ".join([arg.group(4) for arg in func_args])

    cog.outl(f"extern \"C\" {return_type} {func_name}({func_all_args})")
    cog.outl("{")

    cog.outl(f"\tusing FuncType = {return_type} (*)({func_types});")
    cog.outl(f"\tstatic FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym(\"{func_name}\"));")
    cog.outl(f"\treturn realCall({func_names});")

    cog.outl("}")
    cog.outl()
]]]*/
extern "C" lua_State * lua_newstate(lua_Alloc f, void *ud)
{
	using FuncType = lua_State * (*)(lua_Alloc , void *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_newstate"));
	return realCall(f, ud);
}

extern "C" void        lua_close(lua_State *L)
{
	using FuncType = void        (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_close"));
	return realCall(L);
}

extern "C" lua_State * lua_newthread(lua_State *L)
{
	using FuncType = lua_State * (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_newthread"));
	return realCall(L);
}

extern "C" lua_CFunction  lua_atpanic(lua_State *L, lua_CFunction panicf)
{
	using FuncType = lua_CFunction  (*)(lua_State *, lua_CFunction );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_atpanic"));
	return realCall(L, panicf);
}

extern "C" int    lua_gettop(lua_State *L)
{
	using FuncType = int    (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gettop"));
	return realCall(L);
}

extern "C" void   lua_settop(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_settop"));
	return realCall(L, idx);
}

extern "C" void   lua_pushvalue(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushvalue"));
	return realCall(L, idx);
}

extern "C" void   lua_remove(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_remove"));
	return realCall(L, idx);
}

extern "C" void   lua_insert(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_insert"));
	return realCall(L, idx);
}

extern "C" void   lua_replace(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_replace"));
	return realCall(L, idx);
}

extern "C" int    lua_checkstack(lua_State *L, int sz)
{
	using FuncType = int    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_checkstack"));
	return realCall(L, sz);
}

extern "C" void   lua_xmove(lua_State *from, lua_State *to, int n)
{
	using FuncType = void   (*)(lua_State *, lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_xmove"));
	return realCall(from, to, n);
}

extern "C" int              lua_isnumber(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_isnumber"));
	return realCall(L, idx);
}

extern "C" int              lua_isstring(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_isstring"));
	return realCall(L, idx);
}

extern "C" int              lua_iscfunction(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_iscfunction"));
	return realCall(L, idx);
}

extern "C" int              lua_isuserdata(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_isuserdata"));
	return realCall(L, idx);
}

extern "C" int              lua_type(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_type"));
	return realCall(L, idx);
}

extern "C" const char     * lua_typename(lua_State *L, int tp)
{
	using FuncType = const char     * (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_typename"));
	return realCall(L, tp);
}

extern "C" int             lua_equal(lua_State *L, int idx1, int idx2)
{
	using FuncType = int             (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_equal"));
	return realCall(L, idx1, idx2);
}

extern "C" int             lua_rawequal(lua_State *L, int idx1, int idx2)
{
	using FuncType = int             (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_rawequal"));
	return realCall(L, idx1, idx2);
}

extern "C" int             lua_lessthan(lua_State *L, int idx1, int idx2)
{
	using FuncType = int             (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_lessthan"));
	return realCall(L, idx1, idx2);
}

extern "C" lua_Number       lua_tonumber(lua_State *L, int idx)
{
	using FuncType = lua_Number       (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tonumber"));
	return realCall(L, idx);
}

extern "C" lua_Integer      lua_tointeger(lua_State *L, int idx)
{
	using FuncType = lua_Integer      (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tointeger"));
	return realCall(L, idx);
}

extern "C" int              lua_toboolean(lua_State *L, int idx)
{
	using FuncType = int              (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_toboolean"));
	return realCall(L, idx);
}

extern "C" const char     * lua_tolstring(lua_State *L, int idx, size_t *len)
{
	using FuncType = const char     * (*)(lua_State *, int , size_t *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tolstring"));
	return realCall(L, idx, len);
}

extern "C" size_t           lua_objlen(lua_State *L, int idx)
{
	using FuncType = size_t           (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_objlen"));
	return realCall(L, idx);
}

extern "C" lua_CFunction    lua_tocfunction(lua_State *L, int idx)
{
	using FuncType = lua_CFunction    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tocfunction"));
	return realCall(L, idx);
}

extern "C" void           * lua_touserdata(lua_State *L, int idx)
{
	using FuncType = void           * (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_touserdata"));
	return realCall(L, idx);
}

extern "C" lua_State      * lua_tothread(lua_State *L, int idx)
{
	using FuncType = lua_State      * (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_tothread"));
	return realCall(L, idx);
}

extern "C" const void     * lua_topointer(lua_State *L, int idx)
{
	using FuncType = const void     * (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_topointer"));
	return realCall(L, idx);
}

extern "C" void   lua_pushnil(lua_State *L)
{
	using FuncType = void   (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushnil"));
	return realCall(L);
}

extern "C" void   lua_pushnumber(lua_State *L, lua_Number n)
{
	using FuncType = void   (*)(lua_State *, lua_Number );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushnumber"));
	return realCall(L, n);
}

extern "C" void   lua_pushinteger(lua_State *L, lua_Integer n)
{
	using FuncType = void   (*)(lua_State *, lua_Integer );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushinteger"));
	return realCall(L, n);
}

extern "C" void   lua_pushlstring(lua_State *L, const char *s, size_t l)
{
	using FuncType = void   (*)(lua_State *, const char *, size_t );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushlstring"));
	return realCall(L, s, l);
}

extern "C" void   lua_pushstring(lua_State *L, const char *s)
{
	using FuncType = void   (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushstring"));
	return realCall(L, s);
}

extern "C" const char * lua_pushvfstring(lua_State *L, const char *fmt, va_list argp)
{
	using FuncType = const char * (*)(lua_State *, const char *, va_list );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushvfstring"));
	return realCall(L, fmt, argp);
}

extern "C" const char * lua_pushfstring(lua_State *L, const char *fmt, ...)
{
	using FuncType = const char * (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushfstring"));
	return realCall(L, fmt);
}

extern "C" void   lua_pushcclosure(lua_State *L, lua_CFunction fn, int n)
{
	using FuncType = void   (*)(lua_State *, lua_CFunction , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushcclosure"));
	return realCall(L, fn, n);
}

extern "C" void   lua_pushboolean(lua_State *L, int b)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushboolean"));
	return realCall(L, b);
}

extern "C" void   lua_pushlightuserdata(lua_State *L, void *p)
{
	using FuncType = void   (*)(lua_State *, void *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushlightuserdata"));
	return realCall(L, p);
}

extern "C" int    lua_pushthread(lua_State *L)
{
	using FuncType = int    (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pushthread"));
	return realCall(L);
}

extern "C" void   lua_gettable(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gettable"));
	return realCall(L, idx);
}

extern "C" void   lua_getfield(lua_State *L, int idx, const char *k)
{
	using FuncType = void   (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getfield"));
	return realCall(L, idx, k);
}

extern "C" void   lua_rawget(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_rawget"));
	return realCall(L, idx);
}

extern "C" void   lua_rawgeti(lua_State *L, int idx, int n)
{
	using FuncType = void   (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_rawgeti"));
	return realCall(L, idx, n);
}

extern "C" void   lua_createtable(lua_State *L, int narr, int nrec)
{
	using FuncType = void   (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_createtable"));
	return realCall(L, narr, nrec);
}

extern "C" void * lua_newuserdata(lua_State *L, size_t sz)
{
	using FuncType = void * (*)(lua_State *, size_t );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_newuserdata"));
	return realCall(L, sz);
}

extern "C" int    lua_getmetatable(lua_State *L, int objindex)
{
	using FuncType = int    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getmetatable"));
	return realCall(L, objindex);
}

extern "C" void   lua_getfenv(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getfenv"));
	return realCall(L, idx);
}

extern "C" void   lua_settable(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_settable"));
	return realCall(L, idx);
}

extern "C" void   lua_setfield(lua_State *L, int idx, const char *k)
{
	using FuncType = void   (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setfield"));
	return realCall(L, idx, k);
}

extern "C" void   lua_rawset(lua_State *L, int idx)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_rawset"));
	return realCall(L, idx);
}

extern "C" void   lua_rawseti(lua_State *L, int idx, int n)
{
	using FuncType = void   (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_rawseti"));
	return realCall(L, idx, n);
}

extern "C" int    lua_setmetatable(lua_State *L, int objindex)
{
	using FuncType = int    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setmetatable"));
	return realCall(L, objindex);
}

extern "C" int    lua_setfenv(lua_State *L, int idx)
{
	using FuncType = int    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setfenv"));
	return realCall(L, idx);
}

extern "C" void   lua_call(lua_State *L, int nargs, int nresults)
{
	using FuncType = void   (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_call"));
	return realCall(L, nargs, nresults);
}

extern "C" int    lua_pcall(lua_State *L, int nargs, int nresults, int errfunc)
{
	using FuncType = int    (*)(lua_State *, int , int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_pcall"));
	return realCall(L, nargs, nresults, errfunc);
}

extern "C" int    lua_cpcall(lua_State *L, lua_CFunction func, void *ud)
{
	using FuncType = int    (*)(lua_State *, lua_CFunction , void *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_cpcall"));
	return realCall(L, func, ud);
}

extern "C" int    lua_load(lua_State *L, lua_Reader reader, void *dt, const char *chunkname)
{
	using FuncType = int    (*)(lua_State *, lua_Reader , void *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_load"));
	return realCall(L, reader, dt, chunkname);
}

extern "C" int  lua_dump(lua_State *L, lua_Writer writer, void *data)
{
	using FuncType = int  (*)(lua_State *, lua_Writer , void *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_dump"));
	return realCall(L, writer, data);
}

extern "C" int   lua_yield(lua_State *L, int nresults)
{
	using FuncType = int   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_yield"));
	return realCall(L, nresults);
}

extern "C" int   lua_resume(lua_State *L, int narg)
{
	using FuncType = int   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_resume"));
	return realCall(L, narg);
}

extern "C" int   lua_status(lua_State *L)
{
	using FuncType = int   (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_status"));
	return realCall(L);
}

extern "C" int  lua_gc(lua_State *L, int what, int data)
{
	using FuncType = int  (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gc"));
	return realCall(L, what, data);
}

extern "C" int    lua_error(lua_State *L)
{
	using FuncType = int    (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_error"));
	return realCall(L);
}

extern "C" int    lua_next(lua_State *L, int idx)
{
	using FuncType = int    (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_next"));
	return realCall(L, idx);
}

extern "C" void   lua_concat(lua_State *L, int n)
{
	using FuncType = void   (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_concat"));
	return realCall(L, n);
}

extern "C" lua_Alloc  lua_getallocf(lua_State *L, void **ud)
{
	using FuncType = lua_Alloc  (*)(lua_State *, void **);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getallocf"));
	return realCall(L, ud);
}

extern "C" void  lua_setallocf(lua_State *L, lua_Alloc f, void *ud)
{
	using FuncType = void  (*)(lua_State *, lua_Alloc , void *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setallocf"));
	return realCall(L, f, ud);
}

extern "C" void  lua_setlevel(lua_State *from, lua_State *to)
{
	using FuncType = void  (*)(lua_State *, lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setlevel"));
	return realCall(from, to);
}

extern "C" int  lua_getstack(lua_State *L, int level, lua_Debug *ar)
{
	using FuncType = int  (*)(lua_State *, int , lua_Debug *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getstack"));
	return realCall(L, level, ar);
}

extern "C" int  lua_getinfo(lua_State *L, const char *what, lua_Debug *ar)
{
	using FuncType = int  (*)(lua_State *, const char *, lua_Debug *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getinfo"));
	return realCall(L, what, ar);
}

extern "C" const char * lua_getlocal(lua_State *L, const lua_Debug *ar, int n)
{
	using FuncType = const char * (*)(lua_State *, const lua_Debug *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getlocal"));
	return realCall(L, ar, n);
}

extern "C" const char * lua_setlocal(lua_State *L, const lua_Debug *ar, int n)
{
	using FuncType = const char * (*)(lua_State *, const lua_Debug *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setlocal"));
	return realCall(L, ar, n);
}

extern "C" const char * lua_getupvalue(lua_State *L, int funcindex, int n)
{
	using FuncType = const char * (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_getupvalue"));
	return realCall(L, funcindex, n);
}

extern "C" const char * lua_setupvalue(lua_State *L, int funcindex, int n)
{
	using FuncType = const char * (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_setupvalue"));
	return realCall(L, funcindex, n);
}

extern "C" int  lua_sethook(lua_State *L, lua_Hook func, int mask, int count)
{
	using FuncType = int  (*)(lua_State *, lua_Hook , int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_sethook"));
	return realCall(L, func, mask, count);
}

extern "C" lua_Hook  lua_gethook(lua_State *L)
{
	using FuncType = lua_Hook  (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gethook"));
	return realCall(L);
}

extern "C" int  lua_gethookmask(lua_State *L)
{
	using FuncType = int  (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gethookmask"));
	return realCall(L);
}

extern "C" int  lua_gethookcount(lua_State *L)
{
	using FuncType = int  (*)(lua_State *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("lua_gethookcount"));
	return realCall(L);
}

extern "C" void  luaL_openlib(lua_State *L, const char *libname, const luaL_Reg *l, int nup)
{
	using FuncType = void  (*)(lua_State *, const char *, const luaL_Reg *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_openlib"));
	return realCall(L, libname, l, nup);
}

extern "C" void  luaL_register(lua_State *L, const char *libname, const luaL_Reg *l)
{
	using FuncType = void  (*)(lua_State *, const char *, const luaL_Reg *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_register"));
	return realCall(L, libname, l);
}

extern "C" int  luaL_getmetafield(lua_State *L, int obj, const char *e)
{
	using FuncType = int  (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_getmetafield"));
	return realCall(L, obj, e);
}

extern "C" int  luaL_callmeta(lua_State *L, int obj, const char *e)
{
	using FuncType = int  (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_callmeta"));
	return realCall(L, obj, e);
}

extern "C" int  luaL_typerror(lua_State *L, int narg, const char *tname)
{
	using FuncType = int  (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_typerror"));
	return realCall(L, narg, tname);
}

extern "C" int  luaL_argerror(lua_State *L, int numarg, const char *extramsg)
{
	using FuncType = int  (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_argerror"));
	return realCall(L, numarg, extramsg);
}

extern "C" const char * luaL_checklstring(lua_State *L, int numArg, size_t *l)
{
	using FuncType = const char * (*)(lua_State *, int , size_t *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checklstring"));
	return realCall(L, numArg, l);
}

extern "C" const char * luaL_optlstring(lua_State *L, int numArg, const char *def, size_t *l)
{
	using FuncType = const char * (*)(lua_State *, int , const char *, size_t *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_optlstring"));
	return realCall(L, numArg, def, l);
}

extern "C" lua_Number  luaL_checknumber(lua_State *L, int numArg)
{
	using FuncType = lua_Number  (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checknumber"));
	return realCall(L, numArg);
}

extern "C" lua_Number  luaL_optnumber(lua_State *L, int nArg, lua_Number def)
{
	using FuncType = lua_Number  (*)(lua_State *, int , lua_Number );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_optnumber"));
	return realCall(L, nArg, def);
}

extern "C" lua_Integer  luaL_checkinteger(lua_State *L, int numArg)
{
	using FuncType = lua_Integer  (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checkinteger"));
	return realCall(L, numArg);
}

extern "C" lua_Integer  luaL_optinteger(lua_State *L, int nArg, lua_Integer def)
{
	using FuncType = lua_Integer  (*)(lua_State *, int , lua_Integer );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_optinteger"));
	return realCall(L, nArg, def);
}

extern "C" void  luaL_checkstack(lua_State *L, int sz, const char *msg)
{
	using FuncType = void  (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checkstack"));
	return realCall(L, sz, msg);
}

extern "C" void  luaL_checktype(lua_State *L, int narg, int t)
{
	using FuncType = void  (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checktype"));
	return realCall(L, narg, t);
}

extern "C" void  luaL_checkany(lua_State *L, int narg)
{
	using FuncType = void  (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checkany"));
	return realCall(L, narg);
}

extern "C" int    luaL_newmetatable(lua_State *L, const char *tname)
{
	using FuncType = int    (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_newmetatable"));
	return realCall(L, tname);
}

extern "C" void * luaL_checkudata(lua_State *L, int ud, const char *tname)
{
	using FuncType = void * (*)(lua_State *, int , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checkudata"));
	return realCall(L, ud, tname);
}

extern "C" void  luaL_where(lua_State *L, int lvl)
{
	using FuncType = void  (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_where"));
	return realCall(L, lvl);
}

extern "C" int  luaL_error(lua_State *L, const char *fmt, ...)
{
	using FuncType = int  (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_error"));
	return realCall(L, fmt);
}

extern "C" int  luaL_checkoption(lua_State *L, int narg, const char *def, const char *const lst[])
{
	using FuncType = int  (*)(lua_State *, int , const char *, const char *const []);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_checkoption"));
	return realCall(L, narg, def, lst);
}

extern "C" int  luaL_ref(lua_State *L, int t)
{
	using FuncType = int  (*)(lua_State *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_ref"));
	return realCall(L, t);
}

extern "C" void  luaL_unref(lua_State *L, int t, int ref)
{
	using FuncType = void  (*)(lua_State *, int , int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_unref"));
	return realCall(L, t, ref);
}

extern "C" int  luaL_loadfile(lua_State *L, const char *filename)
{
	using FuncType = int  (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_loadfile"));
	return realCall(L, filename);
}

extern "C" int  luaL_loadfilex(lua_State *L, const char *filename, const char *mode)
{
	using FuncType = int  (*)(lua_State *, const char *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_loadfilex"));
	return realCall(L, filename, mode);
}

extern "C" int  luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name)
{
	using FuncType = int  (*)(lua_State *, const char *, size_t , const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_loadbuffer"));
	return realCall(L, buff, sz, name);
}

extern "C" int  luaL_loadstring(lua_State *L, const char *s)
{
	using FuncType = int  (*)(lua_State *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_loadstring"));
	return realCall(L, s);
}

extern "C" lua_State * luaL_newstate()
{
	using FuncType = lua_State * (*)();
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_newstate"));
	return realCall();
}

extern "C" const char * luaL_gsub(lua_State *L, const char *s, const char *p, const char *r)
{
	using FuncType = const char * (*)(lua_State *, const char *, const char *, const char *);
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_gsub"));
	return realCall(L, s, p, r);
}

extern "C" const char * luaL_findtable(lua_State *L, int idx, const char *fname, int szhint)
{
	using FuncType = const char * (*)(lua_State *, int , const char *, int );
	static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("luaL_findtable"));
	return realCall(L, idx, fname, szhint);
}

//[[[end]]] (checksum: b18041df51971df8ce30d0a7be9701f5)
