#include "Datastore.h"
#include "util/util.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

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

BLTFileDataStore* BLTFileDataStore::Open(std::string filePath)
{
	int flags = O_RDONLY;
#ifdef _WIN32
	// Windows Wart - suppress text file conversion
	flags |= O_BINARY;
#endif
	int fd = open(filePath.c_str(), flags);

	// Make sure the file opened correctly
	if (fd == -1)
	{
		return nullptr;
	}

	auto obj = new BLTFileDataStore();
	obj->fd = fd;

	int64_t res = lseek64(fd, 0, SEEK_END);
	assert(res != -1);
	obj->file_size = (size_t)res;

	return obj;
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

	return count;
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
	// TODO this would probably be good to implement if possible
	return false;
}

bool BLTFileDataStore::good() const
{
	PD2HOOK_LOG_ERROR("BLTAbstractDataStore::good called - unimplemented!");
	abort();
}

// BLTStringDataStore

BLTStringDataStore::BLTStringDataStore(std::string contents) : contents(std::move(contents))
{
}

size_t BLTStringDataStore::read(uint64_t position_in_file, uint8_t* data, size_t length)
{
	// If the start of the read is past the end, stop here
	if (position_in_file >= contents.size())
		return 0;

	// If the end of the read is past the end, shrink it down so it'll fit
	size_t remaining = contents.size() - position_in_file;
	if (remaining < length)
		length = remaining;

	memcpy(data, contents.c_str(), length);
	return length;
}

bool BLTStringDataStore::close()
{
	PD2HOOK_LOG_ERROR("BLTStringDataStore::close called - unimplemented!");
	abort();
	// What are we supposed to return?
}

size_t BLTStringDataStore::size() const
{
	return contents.size();
}

bool BLTStringDataStore::is_asynchronous() const
{
	return false;
}

bool BLTStringDataStore::good() const
{
	return true;
}
