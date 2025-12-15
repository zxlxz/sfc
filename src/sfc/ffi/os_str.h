#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

#ifdef _WIN32
using os_char_t = wchar_t;
#else
using os_char_t = char;
#endif

class [[nodiscard]] OsString {
  Vec<os_char_t> _vec = {};

 public:
  explicit OsString(Str s) noexcept;

  auto as_ptr() const noexcept -> const os_char_t* {
    return _vec.as_ptr();
  }
};

}  // namespace sfc::ffi
