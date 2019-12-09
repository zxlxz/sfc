#include "rc.inl"

#include "rc/ffi/cstring.h"

namespace rc::ffi {

pub CString::CString(const Str& s) noexcept
    : _ptr{s.as_ptr()}, _len{s.len()}, _owned{false} {
  if (_len == 0) return;
  if (_ptr[_len] == '\0') return;

  auto p = alloc::alloc<u8>(_len + 1);
  ptr::copy(_ptr, p, _len);
  p[_len] = '\0';

  _ptr = p;
  _owned = true;
}

pub CString::CString(CString&& other) noexcept
    : _ptr{other._ptr}, _len{other._len}, _owned{other._owned} {
  other._owned = false;
  other._ptr = nullptr;
  other._len = 0;
}

pub CString::~CString() {
  if (!_owned) return;

  const auto p = const_cast<u8*>(_ptr);
  alloc::dealloc(p, _len + 1);
}

}  // namespace rc::ffi
