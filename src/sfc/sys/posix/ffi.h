#pragma once

#include "sfc/core.h"

namespace sfc::sys::posix {

auto load_library(const char* path) -> void*;
void unload_library(void* handle);
auto get_symbol(void* handle, const char* name) -> void*;

}  // namespace sfc::sys::posix
