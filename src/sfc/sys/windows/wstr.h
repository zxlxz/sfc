
#pragma once

#include "sfc/alloc.h"

namespace sfc::sys::windows {

using cstr_t = const char*;
using wstr_t = const wchar_t*;

auto cvt_utf8_w16(Slice<const char> in, Slice<wchar_t> out) -> Slice<const wchar_t>;
auto cvt_w16_utf8(Slice<const wchar_t> in, Slice<char> out) -> Slice<const char>;

template <u32 N>
struct FixedWStr {
  wchar_t _buf[N];  // don't init!

 public:
  static auto from_str(Str s) -> FixedWStr {
    FixedWStr res{};
    windows::cvt_utf8_w16({s._ptr, s._len}, res._buf);
    return res;
  }

 public:
  explicit operator bool() const {
    return _buf[0] != L'\0';
  }

  operator wstr_t() const {
    return _buf;
  }
};

}  // namespace sfc::sys::windows
