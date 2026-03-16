#pragma once

// windows.h
#include <Windows.h>

// other system headers
#include <DbgHelp.h>
#include <process.h>
#include <timeapi.h>
#include <winerror.h>
#pragma comment(lib, "DbgHelp.lib")

#undef max
#undef min

#include "sfc/core.h"

namespace sfc::sys::windows {

inline auto os_error() -> DWORD {
  return ::GetLastError();
}

}  // namespace sfc::sys::windows

namespace sfc::sys {
using namespace windows;
}
