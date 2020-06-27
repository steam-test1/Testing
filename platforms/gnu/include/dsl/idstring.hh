#pragma once

#include <string>

namespace dsl
{
	typedef uint64_t idstring_t;

	class idstring
	{
	public:
		idstring_t value;
	};

	struct ResourceID
	{
		idstring_t type = 0;
		idstring_t name = 0;
	};
};

/* vim: set ts=3 softtabstop=0 sw=3 expandtab: */
