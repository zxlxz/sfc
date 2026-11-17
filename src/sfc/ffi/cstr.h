#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

class CString {
  char* _ptr;

 public:
  CString();
  ~CString();
  CString(CString&& other) noexcept;
  CString& operator=(CString&& other) noexcept;

  static auto from_raw(char* ptr) -> CString;
  static auto from_str(Str s) -> CString;

 public:
  auto as_ptr() const -> const char*;
};

}  // namespace sfc::ffi
