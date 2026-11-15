#pragma once

#include "sfc/sys/posix/mod.h"

namespace sfc::sys::posix {

auto alloc(mem::Layout layout) noexcept -> void*;
auto alloc_zeroed(mem::Layout layout) noexcept -> void*;

void dealloc(void* ptr, mem::Layout layout) noexcept;
auto realloc(void* ptr, mem::Layout layout, usize new_size) noexcept -> void*;

}  // namespace sfc::sys::posix
