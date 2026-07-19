#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

auto load_library(const wchar_t* path) -> void*;
void unload_library(void* handle);

auto get_symbol(void* handle, const char* name) -> void*;

}  // namespace sfc::sys::windows
