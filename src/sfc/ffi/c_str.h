#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

class CString {
  using Buf = List<char>;
  Buf _buf;

 public:
  static auto from(Str s) -> CString;

  auto ptr() const -> const char*;
  auto into_string() && -> String;
};

}  // namespace sfc::ffi
