
extern "C" {
#include <sys/stat.h>
}

#include <blt/lapi_compat.hh>
#include <util/util.h>
#include <string.h>
#include <errno.h>
#include <lua.h>
#include <unistd.h>
#include <string>

using namespace blt;

namespace blt
{
	namespace compat
	{
		namespace SystemFS
		{
			int
			exists(lua_state* state)
			{
				// Check arguments
				if(lua_gettop(state) != 2)
					luaL_error(state, "SystemFS:exists(path) takes a single argument, not %d arguments including SystemFS", lua_gettop(state));
				if(!lua_isstring(state, -1))
					luaL_error(state, "SystemFS:exists(path) -> argument 'path' must be a string!");

				// assuming PWD is base folder
				std::string path = lua_tostring(state, -1);

				// If there is a trailing stroke (and this isn't an empty string), ignore it. Sometimes a mod
				// will pass a filepath in with a trailing stroke, and that seems to work on Windows.
				// TODO test the Windows version and double-check this
				while (path.size() > 1 && path.back() == '/')
					path.erase(path.size() - 1);

				struct stat _stat = {};
				int res = stat(path.c_str(), &_stat);
				lua_pushboolean(state, res == 0);
				return 1;
			}

			int delete_file(lua_state* state)
			{
				std::string path = lua_tostring(state, -1);
				bool success = false;

				if (pd2hook::Util::GetFileType(path) == pd2hook::Util::FileType_File)
				{
					success = remove(path.c_str()) == 0;
				}
				else
				{
					success = pd2hook::Util::RemoveDirectory(path);
				}

				if (!success)
				{
					PD2HOOK_LOG_LOG("Could not delete " + path + ": " + strerror(errno));
				}

				lua_pushboolean(state, success);
				return 1;
			}
		}

		void add_members(lua_state* state)
		{
			luaL_Reg lib_SystemFS[] =
			{
				{"delete_file", SystemFS::delete_file},
				{"exists", SystemFS::exists},
				{ NULL, NULL }
			};
			luaL_openlib(state, "SystemFS", lib_SystemFS, 0);
		}
	}
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */
