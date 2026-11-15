#pragma once

#include "sfc/sys/posix/mod.h"

namespace sfc::sys::posix {

using library_t = void*;
using symbol_t = void*;

auto load_library(Str path) -> library_t;
void unload_library(library_t handle);

auto get_symbol(library_t handle, Str name) -> symbol_t;

}  // namespace sfc::sys::posix
