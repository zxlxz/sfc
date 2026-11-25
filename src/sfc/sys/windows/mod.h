#pragma once

#include "sfc/alloc.h"

namespace sfc::sys::windows {

constexpr u32 kMaxPath = 1024;

using cstr_t = const char*;
using wstr_t = const wchar_t*;

auto encode_wstr(Str s, Slice<wchar_t> buf) -> Slice<const wchar_t>;
auto encode_cstr(Slice<const wchar_t> ws, Slice<char> buf) -> cstr_t;
auto make_string(Slice<const wchar_t> ws) -> String;

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
  explicit operator bool() const {
    return _buf[0] != '\0';
  }

  operator cstr_t() const {
    return _buf;
  }
};

template <u32 N>
struct FixedWStr {
  wchar_t _buf[N];

 public:
  static auto from_str(Str s) -> FixedWStr {
    FixedWStr res{};
    windows::encode_wstr(s, res._buf);
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
