
#include "sfc/ffi/cstr.h"

namespace sfc::ffi {

auto CString::from(Str s) -> CString {
  if (s.is_empty()) {
    return {};
  }

  auto res = CString{};
  auto& buf = res._buf;
  buf.reserve(s.len() + 1);
  buf.extend_from_slice(s.as_bytes());
  buf.push(0);
  return res;
}

auto CString::buf() -> Buf& {
  return _buf;
}

auto CString::len() const -> usize {
  const auto n = _buf.len();
  // exclude the trailing null character if exists
  return n == 0 ? 0 : n - 1;
}

auto CString::as_ptr() const -> const char* {
  const auto p = _buf.as_ptr();
  return ptr::cast<char>(p);
}

auto CString::as_cstr() const -> CStr {
  const auto p = this->as_ptr();
  const auto n = this->len();
  return CStr{p, n};
}

void CString::push_str(Str s) {
  if (s._len == 0) {
    return;
  }

  if (!_buf.is_empty()) {
    _buf.pop();
  }

  _buf.extend_from_slice(s.as_bytes());
  _buf.push(0);
}

auto CString::into_string() && -> String {
  auto buf = mem::move(_buf);

  // remove the trailing null character if exists
  if (!buf.is_empty()) {
    buf.pop();
  }
  return String::from_buf(mem::move(buf));
}

}  // namespace sfc::ffi
