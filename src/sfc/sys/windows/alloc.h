#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

auto alloc(mem::Layout layout) noexcept -> void*;
void dealloc(void* ptr, mem::Layout layout) noexcept;
auto realloc(void* ptr, mem::Layout layout, usize new_size) noexcept -> void*;

}  // namespace sfc::sys::windows
