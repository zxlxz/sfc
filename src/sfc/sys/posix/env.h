#pragma once

#include "sfc/sys/posix/mod.h"
#include "sfc/alloc/string.h"

namespace sfc::sys::posix {

auto getenv(Str key) -> String;
auto setenv(Str key, Str val) -> bool;
auto unsetenv(Str key) -> bool;

auto home_dir() -> String;

auto chdir(Str path) -> bool;
auto getcwd() -> String;

}  // namespace sfc::sys::posix
