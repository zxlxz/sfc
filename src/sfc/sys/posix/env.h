#pragma once

#include "sfc/ffi.h"

namespace sfc::sys::posix {

auto getenv(const char* key) -> ffi::CString;
auto setenv(const char* key, const char* val) -> bool;
auto unsetenv(const char* key) -> bool;
auto home_dir() -> ffi::CString;
auto temp_dir() -> ffi::CString;
auto current_exe() -> ffi::CString;
auto getcwd() -> ffi::CString;
auto chdir(const char* path) -> bool;

}  // namespace sfc::sys::posix
