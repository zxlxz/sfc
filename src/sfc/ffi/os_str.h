#pragma once

#include "sfc/ffi/cstr.h"

#ifdef _WIN32
#include "sfc/ffi/wstr.h"
#endif

namespace sfc::ffi {

#ifdef _WIN32
using OsStr = WCStr;
using OsString = WCString;
#else
using OsStr = CStr;
using OsString = CString;
#endif

}  // namespace sfc::ffi
