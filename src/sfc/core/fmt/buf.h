#pragma once

#include "sfc/core/str.h"

namespace sfc::fmt {

template <usize N>
struct Buf {
  char _buf[N];
  usize _len = 0;

 public:
  Buf() = default;

  [[sfc_inline]] void clear() {
    _len = 0;
  }

  [[sfc_inline]] void write_str(str::Str s) {
    if (_len + s._len > N) {
      return;
    }
    __builtin_memcpy(_buf + _len, s._ptr, s._len);
    _len += s._len;
  }

  [[sfc_inline]] auto as_str() const -> str::Str {
    return str::Str{_buf, _len};
  }
};

}  // namespace sfc::fmt
