#pragma once

#include "Datastore.h"
#include "platform.h"

#include <map>
#include <vector>

namespace blt::db
{

	struct DieselBundle
	{
	  public:
		std::string path;
		std::string headerPath;
	};

	struct DslFile
	{
	  public:
		idstring name;
		idstring type;
		int fileId;
		int langId;

		// These are used for reading, and are picked up from the bundle headers
		DieselBundle* bundle = nullptr;
		unsigned int offset = ~0u;
		unsigned int length = ~0u;

		bool Found() const
		{
			return bundle != nullptr;
		}

		bool HasLength() const
		{
			return length != ~0u;
		}

		std::pair<idstring, idstring> Key() const
		{
			return std::pair<idstring, idstring>(name, type);
		}
	};

	class DieselDB
	{
	  private:
		DieselDB();

	  public:
		DieselDB(const DieselDB&) = delete;
		DieselDB& operator=(const DieselDB&) = delete;

		DslFile* Find(idstring name, idstring ext);

		static DieselDB* Instance();

		BLTAbstractDataStore* Open(DieselBundle* bundle);

	  private:
		std::vector<DslFile> filesList;
		std::map<std::pair<idstring, idstring>, DslFile*> files;
	};

}; // namespace blt::db
