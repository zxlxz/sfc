#include "sfc/ffi/cstr.h"

namespace sfc::ffi {

CString::CString() : _ptr{nullptr} {}

CString::~CString() {
  if (_ptr == nullptr) {
    return;
  }
  delete[] _ptr;
}

CString::CString(CString&& other) noexcept : _ptr{other._ptr} {
  other._ptr = nullptr;
}

CString& CString::operator=(CString&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
  }
  return *this;
}

auto CString::from_raw(char* ptr) -> CString {
  auto res = CString{};
  res._ptr = ptr;
  return res;
}

auto CString::from_str(Str s) -> CString {
  if (s._len == 0) {
    return CString{};
  }

  auto p = new char[s._len + 1];
  __builtin_memcpy(p, s._ptr, s._len);
  p[s._len] = '\0';

  return CString::from_raw(p);
}

auto CString::as_ptr() const -> const char* {
  return _ptr == nullptr ? "" : _ptr;
}

}  // namespace sfc::ffi
