
#include "sfc/ffi/c_str.h"

namespace sfc::ffi {

auto CString::from_vec(List<char> v) -> CString {
  auto res = CString{};
  res._buf = mem::move(v);
  return res;
}

auto CString::from(Str s) -> CString {
  if (s.is_empty()) {
    return {};
  }

  auto list = List<char>::with_capacity(s.len() + 1);
  list.extend_from_slice({s._ptr, s._len});
  list.push(0);
  return CString::from_vec(mem::move(list));
}

auto CString::ptr() const -> const char* {
  return _buf.as_ptr();
}

auto CString::into_string() && -> String {
  auto list = reinterpret_cast<List<u8>&&>(_buf);
  if (!list.is_empty()) {
    list.pop();
  }
  return String::from_utf8(mem::move(list));
}

}  // namespace sfc::ffi
