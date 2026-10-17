#pragma once

#include "sfc/alloc/string.h"

namespace sfc::sys::windows {

auto getenv(const wchar_t* key) -> String;
auto setenv(const wchar_t* key, const wchar_t* val) -> bool;
auto unsetenv(const wchar_t* key) -> bool;
auto home_dir() -> String;
auto temp_dir() -> String;
auto current_exe() -> String;
auto getcwd() -> String;
auto chdir(const wchar_t* path) -> bool;

}  // namespace sfc::sys::windows
