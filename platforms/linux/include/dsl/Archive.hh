#pragma once

#include <blt/libcxxstring.hh>

namespace dsl
{
	class DataStore;

	class Archive
	{
	public:
		Archive(std::string const&, dsl::DataStore*, uint64_t, uint64_t, bool, uint64_t);
		Archive(dsl::Archive const&, uint64_t, uint64_t, bool, uint64_t const&);
		Archive(dsl::Archive const&);
		~Archive();

		Archive &operator=(dsl::Archive const&);

		// void set_position(long long, dsl::Archive::SeekMethod);
		void _chk_decompress();
				void checked_read_raw(unsigned char* a, unsigned long b)
		{

			using FuncType = void (*)(Archive*, unsigned char*, unsigned long);
			static FuncType realCall = reinterpret_cast<FuncType>(blt::elf_utils::find_sym("_ZN3dsl7Archive16checked_read_rawEPhm"));

			realCall(this, a, b);
		}
		void checked_write_raw(unsigned char const*, unsigned long);
		void read_ln(std::string*);
		void read_str(std::string*);
		void size() const;

		// Variables
		blt::libcxxstring str;
		size_t position_in_bundle;
		size_t size_var;
		size_t read_counter;
		bool probably_not_loaded_flag;
		char null1[15];
		char lock[40];
		DataStore *datastore;
		uint64_t datastore_safe_counter_id;

		// Add a way to setup these objects from SBLT - we still have to initialise them though, this just makes
		// the compiler happy. Note these are all inline since they don't exist.
		inline explicit Archive() : str("") {};
	};
};

/* vim: set ts=3 softtabstop=0 sw=3 expandtab: */
