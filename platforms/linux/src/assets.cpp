#include <blt/assets.hh>
#include <blt/libcxxstring.hh>
#include <blt/log.hh>

#include <lua.h>
#include <subhook.h>

#include <fstream>
#include <map>
#include <string>

#include <dlfcn.h>
#include <sys/stat.h>

#include <dsl/Archive.hh>
#include <dsl/DB.hh>
#include <dsl/FileSystem.hh>
#include <dsl/Package.hh>
#include <dsl/Transport.hh>

#include <scriptdata/FontData.h>
#include <scriptdata/ScriptData.h>
#include <tweaker/db_hooks.h>

#define hook_remove(hookName) subhook::ScopedHookRemove _sh_remove_raii(&hookName)

PackageManager::find_t PackageManager::find = nullptr;
PackageManager::resource_t PackageManager::resource = nullptr;

using pd2hook::tweaker::dbhook::hook_asset_load;
using namespace std;
using namespace dsl;

namespace blt
{
	// Represents a single custom asset
	class asset_t
	{
	  public:
		// What's the type of the asset - should it be loaded as it is on disk (PLAIN, for most stuff, such
		// as like images, text files, models and so on), or is it a Diesel engine object that needs to be recoded
		// from a 32-bit file (as made on Windows) to a 64-bit one.
		enum asset_type
		{
			PLAIN,
			SCRIPTDATA,
			FONT,
		};

		// The path to the file
		std::string filename;

		// the type, as mentioned above
		asset_type type = PLAIN;
	};

	typedef pair<idstring_t, idstring_t> hash_t;
	static std::map<hash_t, asset_t> custom_assets;

	// LAPI stuff

	namespace lapi
	{
		namespace assets
		{
			int create_entry_ex(lua_state* L)
			{
				luaL_checktype(L, 2, LUA_TUSERDATA);
				luaL_checktype(L, 3, LUA_TUSERDATA);
				luaL_checktype(L, 4, LUA_TSTRING);

				idstring_cls* extension = (idstring_cls*)lua_touserdata(L, 2);
				idstring_cls* name = (idstring_cls*)lua_touserdata(L, 3);
				hash_t hash(name->value, extension->value);

				size_t len;
				const char* filename_c = luaL_checklstring(L, 4, &len);
				string filename(filename_c, len);

				if (custom_assets.count(hash))
				{
					// Appears this is actually allowed in the basegame
					// char buff[1024];
					// snprintf(buff, 1024, "File already exists in replacement DB! %lx.%lx (%s)", name, extension,
					// filename.c_str()); luaL_error(L, buff);
				}

				asset_t asset;
				asset.filename = filename;

				if (lua_type(L, 5) == LUA_TTABLE)
				{
					lua_getfield(L, 5, "recode_type");

					if (lua_isstring(L, -1))
					{
						std::string type = lua_tostring(L, -1);

						if (type == "scriptdata")
						{
							asset.type = asset_t::SCRIPTDATA;
						}
						else if (type == "font")
						{
							asset.type = asset_t::FONT;
						}
						else
						{
							luaL_error(L, "Unknown recode type '%s'", type.c_str());
						}
					}
					lua_pop(L, 1);
				}

				custom_assets[hash] = asset;
				return 0;
			}

			int create_entry(lua_state* L)
			{
				// Chop off anything after the 3rd argument
				if (lua_gettop(L) > 4)
					lua_settop(L, 4);

				return create_entry_ex(L);
			}

			int remove_entry(lua_state* L)
			{
				luaL_checktype(L, 2, LUA_TUSERDATA);
				luaL_checktype(L, 3, LUA_TUSERDATA);

				idstring_cls* extension = (idstring_cls*)lua_touserdata(L, 2);
				idstring_cls* name = (idstring_cls*)lua_touserdata(L, 3);
				hash_t hash(name->value, extension->value);

				custom_assets.erase(hash);

				return 0;
			}

			void setup(lua_state* L)
			{
#define func(name)                \
	lua_pushcclosure(L, name, 0); \
	lua_setfield(L, -2, #name);

				func(create_entry);
				func(remove_entry);

#undef func
			}
		}; // namespace assets
	}; // namespace lapi

	// A datastore that simply provides the contents of a string
	class StringDataStore : public CustomDataStore
	{
	  public:
		virtual ~StringDataStore() override;
		virtual size_t write(size_t position_in_file, uint8_t const* data, size_t length) override;
		virtual size_t read(size_t position_in_file, uint8_t* data, size_t length) override;
		virtual bool close() override;
		virtual size_t size() const override;
		virtual bool is_asynchronous() const override;
		// virtual void set_asynchronous_completion_callback(void* /*dsl::LuaRef*/) {} // ignore this
		// virtual uint64_t state() { return 0; } // ignore this
		virtual bool good() const override;

		std::string contents;
		StringDataStore(std::string contents) : contents(contents)
		{
		}
	};

	// The acual function hooks

	// EACH_HOOK(func): Run a function with an argument ranging from 1 to 4
	// This corresponds to the four usages of templates in the PD2 loading functions
#define EACH_HOOK(func) \
	func(1);            \
	func(2);            \
	func(3);            \
	func(4);            \
	func(5);

	static subhook::Hook
		dslDbAddMembers; // We need this to add the entries, as we need to do it after the DB table is set up

	static void (*dsl_db_add_members)(lua_state*);
	static void (*dsl_fss_open)(Archive* output, FileSystemStack** _this, libcxxstring const*);

	static void (*archive_ctor)(Archive* _this, libcxxstring const& name, dsl::CustomDataStore* datastore,
	                            size_t start_pos, size_t size, bool probably_write_flag, void* ukn);

	typedef void* (*try_open_t)(Archive* target, DB* db, idstring_cls* ext, idstring_cls* name,
	                            void* template_obj /* Misc depends on the template type */, Transport* transport);
	typedef void* (*do_resolve_t)(DB* _this, idstring_cls*, idstring_cls*, void* template_obj, void* unknown);

	// Create variables for each of the functions
	// A detour, and a pointer to the correspoinding try_open and do_resolve functions
#define HOOK_VARS(id)                                  \
	static try_open_t dsl_db_try_open_##id = NULL;     \
	static do_resolve_t dsl_db_do_resolve_##id = NULL; \
	static subhook::Hook dslDbTryOpenDetour##id;

	EACH_HOOK(HOOK_VARS)

	static void open_custom_asset(Archive* target, DB* db, const string& filename, asset_t::asset_type type)
	{
		struct stat buffer = {};
		if (stat(filename.c_str(), &buffer))
		{
			string err = "Cannot open registered asset " + filename;
			log::log(err, log::LOG_ERROR);
			throw err;
		}

		libcxxstring cxxstr = filename; // Use a LibCXX-ABI-compatible string thing

		// Some assets are different on the 32-bit (Windows) and 64-bit (Linux) versions of PAYDAY, so we need to
		// recode them
		if (type != asset_t::PLAIN)
		{
			// Create a datastore. This is what you might call a backing object, which the archive will refer to.
			// Note that the archive will delete the datastore when it's done, so this isn't a memory leak.
			auto* datastore = new StringDataStore("");

			// Read the file in question
			std::ifstream in(filename, std::ios::in | std::ios::binary);
			if (!in)
			{
				// TODO error message
				abort();
			}

			std::string& contents = datastore->contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();

			switch (type)
			{
			case asset_t::SCRIPTDATA:
			{
				pd2hook::scriptdata::ScriptData sd(contents.size(), (const uint8_t*)contents.c_str());
				contents = sd.GetRoot()->Serialise(false);
				break;
			}
			case asset_t::FONT:
			{
				pd2hook::scriptdata::font::FontData fd(contents);
				contents = fd.Export(false);
				break;
			}
			default:
				string msg = "Unknown asset typecode " + to_string(type) +
							 " - this is probably a bug in SuperBLT, please report it";
				throw msg;
			}

			// Create an archive using our datastore, in the memory location passed in (this is how
			// an object is returned in C++ - memory is allocated by the caller, and the pointer is passed
			// in the first argument, even before "this").
			archive_ctor(target, cxxstr, datastore, 0, datastore->size(), false, nullptr);
			return;
		}

		dsl_fss_open(target, &db->stack, &cxxstr);
	}

	static bool try_hook_load(Archive* target, hash_t asset, bool fallback)
	{
		blt::idfile id(asset.first, asset.second);
		BLTAbstractDataStore* datastore = nullptr;
		int64_t pos = 0, len = 0;
		std::string ds_name;
		if (!hook_asset_load(id, &datastore, &pos, &len, ds_name, fallback))
			return false;
		archive_ctor(target, libcxxstring(ds_name), (CustomDataStore*)datastore, pos, len, false, nullptr);
		return true;
	}

	// A generic hook function
	// This can be used with all four of the template values, and it passes everything through to the supplied original
	// function if nothing has changed.
	static void* dt_dsl_db_try_open_hook(Archive* target, DB* db, idstring_cls* ext, idstring_cls* name,
	                                     void* misc_object, Transport* transport, try_open_t original,
	                                     do_resolve_t resolve)
	{
		// TODO caching support!

		hash_t hash(name->value, ext->value);

		// First let Wren override the files
		if (try_hook_load(target, hash, false))
			return target;

		if (custom_assets.count(hash))
		{
			const asset_t& asset = custom_assets[hash];

			open_custom_asset(target, db, asset.filename, asset.type);
			return target;
		}

		original(target, db, ext, name, misc_object, transport);

		// If the asset failed to load, then look for Wren assets again but this time in fallback mode
		if (!target->datastore)
		{
			if (try_hook_load(target, hash, true))
				return target;
		}

		return target;

		/* // Code for doing the same thing as the original function (minus mod_override support):
		int result = resolve(
		    db,
		    ext,
		    name,
		    misc_object,
		    (void*) ((char*)db->ptr4 + 40)
		);

		if ( result < 0 )
		{
		    // Couldn't find the asset
		    *target = Archive("", 0LL, 0LL, 0LL, 1, 0LL);
		}
		else
		{
		    char *ptr = (char*) db;
		#define add_and_dereference(value) ptr = *(char**)(ptr + value)
		    add_and_dereference(80);
		    add_and_dereference(56);
		#undef add_and_dereference
		    unsigned int val = *(unsigned int*) (ptr + 24 + result * 32);
		    transport->vt->f_open(target, transport, val);
		}
		return target;
		*/
	}

	// Create hook functions for each of the original functions
	// These just call the hook function above, passing in the correct function values
#define HOOK_TRY_OPEN(id)                                                                                     \
	static void* dt_dsl_db_try_open_hook_##id(Archive* target, DB* db, idstring_cls* ext, idstring_cls* name, \
	                                          void* misc_object, Transport* transport)                        \
	{                                                                                                         \
		hook_remove(dslDbTryOpenDetour##id);                                                                  \
		return dt_dsl_db_try_open_hook(target, db, ext, name, misc_object, transport, dsl_db_try_open_##id,   \
		                               dsl_db_do_resolve_##id);                                               \
	}
	EACH_HOOK(HOOK_TRY_OPEN)

	// When the members are being added to the DB table, add our own in
	static void dt_dsl_db_add_members(lua_state* L)
	{
		hook_remove(dslDbAddMembers);

		// Make sure we do ours first, so they get overwritten if the basegame
		// implements them in the future
		lapi::assets::setup(L);

		dsl_db_add_members(L);
	}

	// Initialiser function, called by hook.cc
	void init_asset_hook(void* dlHandle)
	{
#define setcall(ptr, symbol) *(void**)(&ptr) = dlsym(dlHandle, #symbol);

		// Get the try_open functions
		setcall(
			dsl_db_try_open_1,
			_ZN3dsl2DB8try_openIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEENS_7ArchiveENS_8idstringESE_RKT_RKNS_9TransportE);
		setcall(dsl_db_try_open_2,
		        _ZN3dsl2DB8try_openIN5sound15EnglishResolverEEENS_7ArchiveENS_8idstringES5_RKT_RKNS_9TransportE);
		setcall(dsl_db_try_open_3,
		        _ZN3dsl2DB8try_openINS_16LanguageResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);
		setcall(dsl_db_try_open_4,
		        _ZN3dsl2DB8try_openINS_21PropertyMatchResolverEEENS_7ArchiveENS_8idstringES4_RKT_RKNS_9TransportE);
		setcall(
			dsl_db_try_open_5,
			_ZN3dsl2DB26try_open_from_bottom_layerIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEENS_7ArchiveENS_8idstringESE_RKT_RKNS_9TransportE);

		// Ge tthe do_resolve functions
		setcall(
			dsl_db_do_resolve_1,
			_ZNK3dsl2DB10do_resolveIFiRKNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS4_EENS_9AllocatorEEEiiEEEiNS_8idstringESD_RKT_PS9_);
		setcall(
			dsl_db_do_resolve_2,
			_ZNK3dsl2DB10do_resolveIN5sound15EnglishResolverEEEiNS_8idstringES4_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessISA_EENS_9AllocatorEEE);
		setcall(
			dsl_db_do_resolve_3,
			_ZNK3dsl2DB10do_resolveINS_16LanguageResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);
		setcall(
			dsl_db_do_resolve_4,
			_ZNK3dsl2DB10do_resolveINS_21PropertyMatchResolverEEEiNS_8idstringES3_RKT_PNS_7SortMapINS_5DBExt3KeyEjNSt3__14lessIS9_EENS_9AllocatorEEE);

		// The misc. functions
		setcall(dsl_db_add_members, _ZN3dsl6MainDB11add_membersEP9lua_State);
		setcall(dsl_fss_open,
		        _ZNK3dsl15FileSystemStack4openERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE);

		// The archive constructor
		setcall(
			archive_ctor,
			_ZN3dsl7ArchiveC1ERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPNS_9DataStoreExxbx);

		// The unit creation hooking
		setcall(PackageManager::find,
		        _ZN14PackageManager4findERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE);
		setcall(PackageManager::resource, _ZN14PackageManager8resourceERKN3dsl10ResourceIDE);
#undef setcall

		// Add the 'add_members' hook
		dslDbAddMembers.Install((void*)dsl_db_add_members, (void*)dt_dsl_db_add_members,
		                        subhook::HookOption64BitOffset);

		// Hook each of the four loading functions
#define INSTALL_TRY_OPEN_HOOK(id)                                                                    \
	dslDbTryOpenDetour##id.Install((void*)dsl_db_try_open_##id, (void*)dt_dsl_db_try_open_hook_##id, \
	                               subhook::HookOption64BitOffset);
		EACH_HOOK(INSTALL_TRY_OPEN_HOOK)
	}

	void asset_add_lua_members(lua_State* L)
	{
		lua_getglobal(L, "blt");
		lua_pushcfunction(L, lapi::assets::create_entry_ex);
		lua_setfield(L, -2, "db_create_entry");
		lua_pop(L, 1);
	}

	// StringDataStore implementation
	StringDataStore::~StringDataStore()
	{
	}

	size_t StringDataStore::write(size_t position_in_file, uint8_t const* data, size_t length)
	{
		// simply not supported, TODO handle this better?
		abort();
	}

	size_t StringDataStore::read(size_t position_in_file, uint8_t* data, size_t length)
	{
		size_t real_len = min(length, size() - position_in_file);

		if (real_len <= 0)
			return 0;

		std::copy(contents.begin() + position_in_file, contents.begin() + real_len, data);
		return real_len;
	}

	bool StringDataStore::close()
	{
		// TODO clear out the string and make good() return false

		// afaik we just should always return true?
		return true;
	}

	size_t StringDataStore::size() const
	{
		return contents.length();
	}

	bool StringDataStore::is_asynchronous() const
	{
		return false;
	}

	bool StringDataStore::good() const
	{
		return true;
	}
}; // namespace blt

/* vim: set ts=4 sw=4 noexpandtab: */
