#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

class [[nodiscard]] CString {
  Vec<char> _vec = {};

 public:
  static auto xnew(Str s) noexcept -> CString {
    auto res = CString{};
    res._vec.reserve(s._len + 1);
    res._vec.extend_from_slice({s._ptr, s._len});
    res._vec.push('\0');
    return res;
  }

  auto as_ptr() const noexcept -> cstr_t {
    return _vec.as_ptr();
  }
};

}  // namespace sfc::ffi
