#pragma once

#include "sfc/alloc.h"

namespace sfc::ffi {

class [[nodiscard]] CString {
  Vec<char> _inn{};

  CString(Vec<char> v) noexcept : _inn{mem::move(v)} {
    if (_inn && _inn.last() != 0) {
      _inn.reserve_extract(1);
      _inn.push(0);
    }
  }

 public:
  CString() = default;

  ~CString() = default;

  CString(CString&& other) noexcept = default;

  CString& operator=(CString&&) noexcept = default;

  static auto from(Str s) -> CString {
    return CString{Vec<char>::from(s.as_chars())};
  }

  static auto from_vec(Vec<char> v) -> CString {
    return CString{mem::move(v)};
  }

  static auto from_string(String s) -> CString {
    return CString{mem::move(s.as_mut_vec())};
  }

  static auto from_raw(const char* p) -> CString {
    return CString{Vec<char>::from(Str{p}.as_chars())};
  }

  auto as_str() const -> Str {
    if (!_inn) {
      return {};
    }
    return {_inn.as_ptr(), _inn.len() - 1};
  }

  operator cstr_t() const {
    if (!_inn) {
      return nullptr;
    }
    return _inn.as_ptr();
  }
};

}  // namespace sfc::ffi
