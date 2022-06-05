#include "blt/libcxxstring.hh"
#include <string.h>

using namespace std;
using namespace blt;

// Main Constructors
libcxxstring::libcxxstring(const char *value) : str(0)
{
	set_contents(value, strlen(value));
}

libcxxstring::libcxxstring(const char *value, size_t length) : str(0)
{
	set_contents(value, length);
}

libcxxstring::libcxxstring(std::string value) : str(0)
{
	set_contents(value.c_str(), value.length());
}

// Destructor
libcxxstring::~libcxxstring()
{
	delete_str();
}

// Copy constructor and assignment operator
libcxxstring::libcxxstring(libcxxstring &other) : str(0)
{
	// Pass it on to the assignment operator
	*this = other;
}
libcxxstring& libcxxstring::operator=(const libcxxstring &other)
{
	set_contents(other.str, other.size);
	return *this;
}

// Conversion to C and stdlib strings
libcxxstring::operator std::string() const
{
	return string(str, size);
}

const char* libcxxstring::c_str() const
{
	return str;
}

// Set the value of this string - the bulk of the class
void libcxxstring::set_contents(const char *src, size_t length)
{
	// Clear the string
	delete_str();

	// Create our text array
	str = new char[length];
	size = length;

	// Or the length with 1 to mark this as a long string
	capacity_and_flags = length | 1;

	// Copy it in
	memcpy(str, src, length);
}

void libcxxstring::delete_str()
{
	if (str && (capacity_and_flags & 1) != 0)
	{
		delete str;
		str = nullptr;
	}
}
