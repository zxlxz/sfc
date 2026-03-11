#pragma once

#include "sfc/alloc/vec.h"
#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct WChars {
  const wchar_t* _ptr;
  const wchar_t* _end;

 public:
  auto next() noexcept -> Option<char32_t>;
};

class WString {
  Vec<wchar_t> _vec;

 public:
  static auto from(Str s) -> WString;
  static auto from_vec(Vec<wchar_t> v) -> WString;

  auto ptr() const -> const wchar_t*;
  auto chars() const -> WChars;

  auto into_string() const -> String;
};

}  // namespace sfc::ffi
