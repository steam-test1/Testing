#pragma once

#include "platform.h"
#include <string>

namespace blt
{
	namespace plugins
	{
		namespace environment
		{
			bool isVr();
			const char* getModDirectory(const char* nativeModulePath);

			unsigned long long hashString(const char* str);
		} // namespace environment
	} // namespace plugins
} // namespace blt