#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct WStr {
  const wchar_t* _ptr;
  usize _len;

 public:
  auto to_string() const -> String;
};

struct WCStr {
  const wchar_t* _inn;

 public:
  WCStr(const wchar_t* p) : _inn{p} {}

 public:
  auto to_string() const -> String;
};

class WCString {
  List<wchar_t> _buf;

 public:
  static auto from_str(Str s) -> WCString;

 public:
  auto as_ptr() const -> const wchar_t*;

  void push(char32_t c);
  void push_str(Str s);

  auto into_string() const -> String;
};

}  // namespace sfc::ffi
