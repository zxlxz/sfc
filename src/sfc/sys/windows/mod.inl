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

template <class F>
struct VecType;

template <class R, class V>
struct VecType<R(V)> {
  using Vec = V;
};

template <class S = WString>
static auto build_string(auto&& f, unsigned capacity = 0) -> S {
  using V = typename VecType<decltype(S::from_vec)>::Vec;

  if (capacity == 0) {
    capacity = f(nullptr, 0);
  }

  if (capacity == 0) {
    return {};
  }

  auto vec = V::with_capacity(capacity);
  const auto len = f(vec.as_mut_ptr(), capacity);
  vec.set_len(len + 1);

  return S::from_vec(static_cast<V&&>(vec));
}

}  // namespace sfc::sys
