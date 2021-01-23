#pragma once

#include "platform.h"
#include "xmltweaker.h"
#include <string>

namespace pd2hook
{
	namespace tweaker
	{
		/**
		 * Transforms the contents of the file
		 * the return result MUST be valid immediately after the call returns,
		 * but not necessaraly after that.
		 */
		const char* transform_file(const char* contents);
	}; // namespace tweaker
}; // namespace pd2hook
