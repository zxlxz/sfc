#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct WChars {
  const wchar_t* _ptr;
  const wchar_t* _end;

 public:
  auto next() noexcept -> Option<char32_t>;
};

class WString {
  using Buf = List<wchar_t>;
  Buf _buf;

 public:
  static auto from(Str s) -> WString;

  auto ptr() const -> const wchar_t*;
  auto chars() const -> WChars;

  auto into_string() const -> String;
};

}  // namespace sfc::ffi
