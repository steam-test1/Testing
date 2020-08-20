#include "Datastore.h"
#include "util/util.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#define lseek64 _lseeki64
#else
#include <unistd.h>
#endif

// BLTAbstractDataStore

size_t BLTAbstractDataStore::write(uint64_t position_in_file, uint8_t const* data, size_t length)
{
	// Writing is unsupported
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::write called - writing is not supported!");
	abort();
}

void BLTAbstractDataStore::set_asynchronous_completion_callback(void* /*dsl::LuaRef*/)
{
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::set_asynchronous_completion_callback called - async unimplemented!");
	abort();
}

uint64_t BLTAbstractDataStore::state()
{
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::state called - unimplemented!");
	abort();
}

// BLTFileDataStore

BLTFileDataStore::BLTFileDataStore(std::string filePath)
{
	int flags = O_RDONLY;
#ifdef _WIN32
	// Windows Wart - suppress text file conversion
	flags |= O_BINARY;
#endif
	fd = open(filePath.c_str(), flags);
	assert(fd != -1); // Make sure the file opened correctly

	int64_t res = lseek64(fd, 0, SEEK_END);
	assert(res != -1);
	file_size = res;
}

BLTFileDataStore::~BLTFileDataStore()
{
	::close(fd);
}

size_t BLTFileDataStore::read(uint64_t position_in_file, uint8_t* data, size_t length)
{
	lseek64(fd, position_in_file, SEEK_SET);
	size_t count = ::read(fd, data, length);
	assert(count == length);

	return length;
}

bool BLTFileDataStore::close()
{
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::close called - unimplemented!");
	abort();
}

size_t BLTFileDataStore::size() const
{
	return file_size;
}

bool BLTFileDataStore::is_asynchronous() const
{
	return false;
}

bool BLTFileDataStore::good() const
{
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::good called - unimplemented!");
	abort();
}
