#pragma once

#include <wren.hpp>

#include <mutex>

namespace pd2hook::wren
{
	WrenVM* get_wren_vm();
	std::lock_guard<std::recursive_mutex> lock_wren_vm();
} // namespace pd2hook::wren
