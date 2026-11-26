#pragma once

#include "sfc/core.h"

namespace sfc::sys {

using cstr_t = const char*;

template <u32 N>
struct FixedCStr {
  char _buf[N];

 public:
  static auto from_str(Str s) -> FixedCStr {
    const auto n = s._len < N ? s._len : N - 1;

    FixedCStr res{};
    __builtin_memcpy(res._buf, s._ptr, n);
    res._buf[n] = '\0';
    return res;
  }

 public:
  explicit operator bool() const {
    return _buf[0] != '\0';
  }

  operator cstr_t() const {
    return _buf;
  }
};

}  // namespace sfc::sys
