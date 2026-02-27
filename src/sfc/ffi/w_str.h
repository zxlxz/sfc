#pragma once

#include "sfc/alloc/vec.h"
#include "sfc/alloc/string.h"

namespace sfc::ffi {

class WString {
  Vec<wchar_t> _vec;

 public:
  static auto from(Str s) -> WString;

  auto ptr() const -> const wchar_t* {
    return _vec.as_ptr();
  }

  auto as_mut_vec() -> Vec<wchar_t>& {
    return _vec;
  }

  auto into_string() && -> String;
};

}  // namespace sfc::ffi
