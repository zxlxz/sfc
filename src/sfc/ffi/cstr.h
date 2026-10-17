#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct CStr {
  const char* _inn;

 public:
  CStr(const char* p) : _inn{p} {}

 public:
  auto to_str() const -> Str;
  auto to_string() const -> String;
};

class CString {
  List<char> _buf;

 public:
  static auto from(CStr s) -> CString;
  static auto from_str(Str s) -> CString;

 public:
  auto as_ptr() const -> const char*;
  auto as_cstr() const -> CStr;

  void push_str(Str s);
  auto into_string() && -> String;
};

}  // namespace sfc::ffi
