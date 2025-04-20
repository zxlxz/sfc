#pragma once

#include "sfc/alloc.h"

namespace sfc::ffi {

class CString {
  Vec<char> _vec = {};

 public:
  CString() = default;
  ~CString() = default;
  CString(CString&&) noexcept = default;
  CString& operator=(CString&&) noexcept = default;

  static auto from(Str s) -> CString {
    auto res = CString{};
    if (!s.is_empty()) {
      res._vec.extend_from_slice(s.as_chars());
      res._vec.push('\0');
    }
    return res;
  }

  auto c_str() const -> const char* {
    return _vec.as_ptr();
  }
};

}  // namespace sfc::ffi
