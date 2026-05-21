#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct CStr {
  const char* _ptr;
  usize _len;
};

class CString {
  using Buf = List<u8>;
  Buf _buf;

 public:
  static auto from(Str s) -> CString;

  auto buf() -> Buf&;
  auto len() const -> usize;
  auto as_ptr() const -> const char*;
  auto as_cstr() const -> CStr;

 public:
  void push_str(Str s);
  auto into_string() && -> String;
};

}  // namespace sfc::ffi
