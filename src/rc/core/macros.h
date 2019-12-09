#pragma once

#include "rc/core/fmt.h"
#include "rc/core/panicking.h"

namespace rc {

template <class... T>
inline auto panic(const T&... args) -> void {
  if constexpr (sizeof...(T) == 0) {
    return;
  } else if constexpr (sizeof...(T) == 1) {
    panicking::panic_str(args...);
  } else {
    panicking::panic_fmt(fmt::Args{args...});
  }
}

template <class... Args>
inline auto assert(bool cond, const Args&... args) -> void {
  if (cond) {
    return;
  }
  rc::panic(u8"assert failed: {}", fmt::Args{args...});
}

template <class A, class B>
inline auto assert_eq(const A& a, const B& b) -> void {
  if (a == b) {
    return;
  }
  rc::panic(u8"assert failed: `(a == b)`, a=`{}`, b=`{}`",a, b);
}

template <class A, class B>
inline auto assert_ne(const A& a, const B& b) -> void {
  if (a != b) {
    return;
  }
  rc::panic(u8"assert failed: `(a != b)`, a=`{}`, b=`{}`", a, b);
}

}  // namespace rc
