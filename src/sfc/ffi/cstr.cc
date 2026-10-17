#include "sfc/ffi/cstr.h"

namespace sfc::ffi {

auto CStr::to_str() const -> Str {
  const auto n = _inn ? __builtin_strlen(_inn) : 0;
  return Str{_inn, n};
}

auto CStr::to_string() const -> String {
  const auto s = this->to_str();
  return String::from(s);
}

auto CString::from(CStr s) -> CString {
  auto res = CString{};
  res.push_str(s.to_str());
  return res;
}

auto CString::from_str(Str s) -> CString {
  auto res = CString{};
  res.push_str(s);
  return res;
}

auto CString::as_ptr() const -> const char* {
  const auto p = _buf.as_ptr();
  return ptr::cast<const char>(p);
}

void CString::push_str(Str s) {
  if (s._len == 0) {
    return;
  }

  if (!_buf.is_empty()) {
    _buf.pop();
  }

  _buf.extend_from_slice({s._ptr, s._len});
  _buf.push(0);
}

auto CString::into_string() && -> String {
  // remove the trailing null character if exists
  if (!_buf.is_empty()) {
    _buf.pop();
  }

  auto& u8_buf = *ptr::cast<List<u8>>(&_buf);
  return String::from_utf8(mem::move(u8_buf));
}

}  // namespace sfc::ffi
