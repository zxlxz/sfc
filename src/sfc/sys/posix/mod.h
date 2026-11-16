#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

using cstr_t = const char*;

template <u32 N>
struct FixedCStr {
  char _buf[N];

 public:
  static auto from_str(Str s) -> FixedCStr {
    FixedCStr res{};
    if (s._len < N - 1) {
      __builtin_memcpy(res._buf, s._ptr, s._len);
      res._buf[s._len] = '\0';
    } else {
      res._buf[0] = '\0';
    }
    return res;
  }

 public:
  operator cstr_t() const {
    return _buf;
  }
};

}  // namespace sfc::sys::windows
