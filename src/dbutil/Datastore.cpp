#include "Datastore.h"

#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>

// BLTAbstractDataStore

size_t BLTAbstractDataStore::write(size_t position_in_file, uint8_t const *data, size_t length)
{
	// Writing is unsupported
	abort();
}

void BLTAbstractDataStore::set_asynchronous_completion_callback(void* /*dsl::LuaRef*/)
{
	abort();
}

uint64_t BLTAbstractDataStore::state()
{
	abort();
}

// BLTFileDataStore

BLTFileDataStore::BLTFileDataStore(std::string filePath)
{
	fd = open(filePath.c_str(), O_RDONLY);
	assert(fd != -1); // Make sure the file opened correctly
}

BLTFileDataStore::~BLTFileDataStore()
{
	::close(fd);
}

size_t BLTFileDataStore::read(size_t position_in_file, uint8_t * data, size_t length)
{
	lseek(fd, position_in_file, SEEK_SET);
	size_t count = ::read(fd, data, length);
	assert(count == length);

	return length;
}

bool BLTFileDataStore::close()
{
	abort();
}

size_t BLTFileDataStore::size() const
{
	abort();
}

bool BLTFileDataStore::is_asynchronous() const
{
	return false;
}

bool BLTFileDataStore::good() const
{
	abort();
}
