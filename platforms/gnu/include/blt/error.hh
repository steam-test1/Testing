#include <string>

#include "lua.hh"

namespace blt
{
	namespace error
	{
		int check_callback(lua_state*);
		void set_global_handlers();
	}
}

/* vim: set ts=4 softtabstop=0 sw=4 expandtab: */

