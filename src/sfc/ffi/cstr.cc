#include "sfc/ffi/cstr.h"

namespace sfc::ffi {

CString::CString() : _ptr{nullptr} {}

CString::~CString() {
  if (_ptr == nullptr) {
    return;
  }
  __builtin_free(_ptr);
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

auto CString::from_str(Str s) -> CString {
  static constexpr u32 kMaxCStrLen = 1 * 1024 * 1024;  // 1MB
  if (s._len == 0 || s._len > kMaxCStrLen) {
    return {};
  }

  auto res = CString{};
  res._ptr = static_cast<char*>(__builtin_malloc(s._len + 1));
  if (res._ptr == nullptr) {
    return {};
  }

  __builtin_memcpy(res._ptr, s._ptr, s._len);
  res._ptr[s._len] = '\0';
  return res;
}

auto CString::as_ptr() const -> const char* {
  return _ptr == nullptr ? "" : _ptr;
}

}  // namespace sfc::ffi
