#pragma once

#include "sfc/sys/windows/mod.h"

namespace sfc::sys::windows {

using module_t = void*;
using symbol_t = isize (*)();

auto load_library(Str path) -> module_t;
void unload_library(module_t handle);

auto get_symbol(module_t handle, Str name) -> symbol_t;

}  // namespace sfc::sys::windows
