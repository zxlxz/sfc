#pragma once

#include "sfc/ffi.h"

namespace sfc::sys::windows {

auto getenv(const wchar_t* key) -> ffi::WString;
auto setenv(const wchar_t* key, const wchar_t* val) -> bool;
auto unsetenv(const wchar_t* key) -> bool;
auto home_dir() -> ffi::WString;
auto temp_dir() -> ffi::WString;
auto current_exe() -> ffi::WString;
auto getcwd() -> ffi::WString;
auto chdir(const wchar_t* path) -> bool;

}  // namespace sfc::sys::windows
