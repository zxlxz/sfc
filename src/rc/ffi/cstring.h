#pragma once

#include "rc/alloc.h"

namespace rc::ffi {

struct CString {
  const u8* _ptr;
  usize _len;
  bool _owned;

  pub explicit CString(const Str& s) noexcept;
  pub CString(CString&& other) noexcept;
  pub ~CString();

  operator const char*() const noexcept {
    const auto p = ptr::cast<char>(_ptr);
    return p;
  }
};

}  // namespace rc::ffi
