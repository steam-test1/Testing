#pragma once

#include <blt/libcxxstring.hh>
#include <dsl/idstring.hh>

class Package
{
public:
	bool preload(const dsl::ResourceID &resource, bool force_generic_allocator, bool prob_load_textures);
};

class PackageManager
{
public:
	static PackageManager* get();

	// Helpers, set in assets:

	// Can't link since it expects std::string, won't work on glibc
	typedef Package**(*find_t)(PackageManager *self, const blt::libcxxstring &str);
	static find_t find;

	// Unfortunately C++ doesn't let you cast a member function pointer to void*, so we
	// have to hack around it like so:
	typedef void*(*resource_t)(PackageManager *self, const dsl::ResourceID &);
	static resource_t resource;
};
