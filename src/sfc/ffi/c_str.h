#pragma once

#include "sfc/alloc/vec.h"
#include "sfc/alloc/string.h"

namespace sfc::ffi {

class CString {
  Vec<char> _vec;

 public:
  static auto from(Str s) -> CString;

  auto ptr() const -> const char* {
    return _vec.as_ptr();
  }

  auto into_string() && -> String;
};

}  // namespace sfc::ffi
