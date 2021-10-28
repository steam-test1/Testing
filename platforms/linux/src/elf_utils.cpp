#include "blt/elf_utils.hh"

extern "C" {
#include <elf.h>
}
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cassert>

static_assert(__x86_64__, "We only support 64 bit ELFs here");

static bool
read_own_exe(std::vector<char>& data)
{
	std::ifstream file("/proc/self/exe", std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	data.resize(size);
	return !file.read(data.data(), size).fail();
}

static inline const Elf64_Shdr*
get_elf_section_header_by_index(const Elf64_Ehdr* elfHeader, size_t index)
{
	return ((Elf64_Shdr*) ((uintptr_t) elfHeader + (uintptr_t) elfHeader->e_shoff)) + index;
}

/**
 * Searches for the first section header in an ELF file with a given name
 *
 * Returns nullptr if the header is not found
 */
static const Elf64_Shdr*
find_elf_section_header_by_name(const Elf64_Ehdr* elfHeader, const char* name)
{
	const Elf64_Shdr* shstrtabHeader = get_elf_section_header_by_index(elfHeader, elfHeader->e_shstrndx);
	const char* shstrtab = (const char*) elfHeader + shstrtabHeader->sh_offset;

	// Loop through all sections in the ELF
	for (size_t i = 0; i < elfHeader->e_shnum; i++)
	{
		const Elf64_Shdr* sectionHeader = get_elf_section_header_by_index(elfHeader, i);

		if (strcmp(shstrtab + sectionHeader->sh_name, name) == 0)
		{
			return sectionHeader;
		}
	}

	return nullptr;
}

/**
 * Finds a symbol by its name
 *
 * Returns nullptr if the symbol is not found
 */
static const Elf64_Sym*
find_elf_symbol_by_name(const Elf64_Ehdr* elfHeader, const Elf64_Shdr* symtabHeader, const Elf64_Shdr* strtabHeader, const char* name)
{
	const Elf64_Sym* symtab = (Elf64_Sym*) ((char*) elfHeader + symtabHeader->sh_offset);
	const char* strtab = (char*) elfHeader + strtabHeader->sh_offset;

	// Loop through all symbols in the ELF
	for (const char* symPtr = (const char*) symtab; symPtr < (const char*) symtab + symtabHeader->sh_size; symPtr += symtabHeader->sh_entsize)
	{
		const Elf64_Sym* sym = (Elf64_Sym*) symPtr;
		const char* symName = strtab + sym->st_name;

		if (strcmp(symName, name) == 0)
		{
			// We found the symbol
			return sym;
		}
	}

	return nullptr;
}

static std::vector<char> ownElf;
static const Elf64_Ehdr* paydayElfHeader = nullptr;
static const Elf64_Shdr* paydaySymtabHeader = nullptr;
static const Elf64_Shdr* paydayStrtabHeader = nullptr;

namespace blt
{
	namespace elf_utils
	{
		void init()
		{
			bool success = read_own_exe(ownElf);
			assert(success && "Failed to read /proc/self/exe into a buffer");

			const Elf64_Ehdr* elfHeader = (Elf64_Ehdr*) ownElf.data();

			// Check for the ELF magic number
			assert(
					elfHeader->e_ident[0] == ELFMAG0 &&
					elfHeader->e_ident[1] == ELFMAG1 &&
					elfHeader->e_ident[2] == ELFMAG2 &&
					elfHeader->e_ident[3] == ELFMAG3 &&
					"Found the base address of the PAYDAY 2 ELF, but it doesn't seem to have the ELF magic number?"
				  );

			// Search for the .symtab and .strtab section headers
			const Elf64_Shdr* symtabHeader = find_elf_section_header_by_name(elfHeader, ".symtab");
			assert(symtabHeader != nullptr && "Couldn't find .symtab ELF section header");

			const Elf64_Shdr* strtabHeader = find_elf_section_header_by_name(elfHeader, ".strtab");
			assert(strtabHeader != nullptr && "Couldn't find .strtab ELF section header");

			paydayElfHeader = elfHeader;
			paydaySymtabHeader = symtabHeader;
			paydayStrtabHeader = strtabHeader;
		}

		void* find_sym(const char* name)
		{
			assert(paydayElfHeader != nullptr && "You forgot to call blt::elf_utils::init()");
			assert(paydaySymtabHeader != nullptr && "You forgot to call blt::elf_utils::init()");
			assert(paydayStrtabHeader != nullptr && "You forgot to call blt::elf_utils::init()");

			const Elf64_Sym* sym = find_elf_symbol_by_name(
					paydayElfHeader,
					paydaySymtabHeader,
					paydayStrtabHeader,
					name
					);
			if (sym == nullptr) return nullptr;

			return (void*) sym->st_value;
		}
	}
}
