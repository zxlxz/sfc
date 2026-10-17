#pragma once

#include "sfc/ffi/cstr.h"

#ifdef _WIN32
#include "sfc/ffi/wstr.h"
#endif

namespace sfc::ffi {

#ifdef _WIN32
using OsStr = U16Str;
using OsString = U16String;
#else
using OsStr = CStr;
using OsString = CString;
#endif

}  // namespace sfc::ffi
