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
#include "sfc/ffi.h"

namespace sfc::sys::windows {

using ffi::WString;

static auto build_wstring(auto&& f, unsigned capacity = 0) -> WString {
  if (capacity == 0) {
    capacity = f(nullptr, 0);
  }

  if (capacity == 0) {
    return {};
  }

  auto vec = Vec<wchar_t>::with_capacity(capacity);
  const auto len = f(vec.as_mut_ptr(), capacity);
  vec.set_len(len + 1);

  return WString::from_vec(mem::move(vec));
}

static inline auto os_error() -> DWORD {
  return ::GetLastError();
}

}  // namespace sfc::sys::windows

namespace sfc::sys {
using namespace windows;
}
