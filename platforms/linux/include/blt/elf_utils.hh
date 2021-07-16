#pragma once


namespace blt
{
	namespace elf_utils
	{
		void init();
		void* find_sym(const char* name);
	}
}
