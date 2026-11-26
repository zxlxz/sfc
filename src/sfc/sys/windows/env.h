#pragma once

#include "sfc/alloc/string.h"

namespace sfc::sys::windows {

auto getenv(Str key) -> String;
auto setenv(Str key, Str val) -> bool;
auto unsetenv(Str key) -> bool;

auto home_dir() -> String;

auto getcwd() -> String;
auto chdir(Str path) -> bool;

}  // namespace sfc::sys::windows
