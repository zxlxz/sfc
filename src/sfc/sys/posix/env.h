#pragma once

#include "sfc/alloc/string.h"

namespace sfc::sys::posix {

auto getenv(const char* key) -> String;
auto setenv(const char* key, const char* val) -> bool;
auto unsetenv(const char* key) -> bool;
auto home_dir() -> String;
auto temp_dir() -> String;
auto current_exe() -> String;
auto getcwd() -> String;
auto chdir(const char* path) -> bool;

}  // namespace sfc::sys::posix
