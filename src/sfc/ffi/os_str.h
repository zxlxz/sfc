#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

class CString {
  using Buf = List<u8>;
  Buf _buf;

 public:
  static auto from(Str s) -> CString;

  auto ptr() const -> const char*;
  auto into_string() && -> String;
};

class WString {
  using Buf = List<wchar_t>;
  Buf _buf;

 public:
  static auto from(Str s) -> WString;

  auto ptr() const -> const wchar_t*;
  auto buf() -> Buf&;
  auto into_string() const -> String;
};


#ifdef _WIN32
using OsString = WString;
#else
using OsString = CString;
#endif

}  // namespace sfc::ffi
