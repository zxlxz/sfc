#pragma once

#include "sfc/ffi/c_str.h"
#include "sfc/ffi/w_str.h"

namespace sfc::ffi {

#ifdef _WIN32
using OsString = WString;
#else
using OsString = CString;
#endif

}  // namespace sfc::ffi
