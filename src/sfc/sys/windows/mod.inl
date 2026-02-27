#pragma once

// windows.h
#include <Windows.h>

// other system headers
#include <DbgHelp.h>
#include <process.h>
#include <timeapi.h>
#include <winerror.h>

#pragma comment(lib, "DbgHelp.lib")

namespace sfc::ffi {
class WString;
}

namespace sfc::sys {

using ffi::WString;

template <class S = WString>
static auto build_string(auto&& f, unsigned capacity = 0) -> S {
  if (capacity == 0) {
    capacity = f(nullptr, 0);
  }

  if (capacity == 0) {
    return {};
  }

  auto res = S{};
  auto& vec = res.as_mut_vec();
  vec.reserve(capacity);

  const auto len = f(vec.as_mut_ptr(), capacity);
  vec.set_len(len);

  return res;
}

}  // namespace sfc::sys
