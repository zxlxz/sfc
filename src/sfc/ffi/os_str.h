#pragma once

#include "sfc/ffi/cstr.h"

#ifdef _WIN32
#include "sfc/ffi/wstr.h"
#endif

namespace sfc::ffi {

#ifdef _WIN32
using OsString = WString;
#else
using OsString = CString;
#endif

}  // namespace sfc::ffi
