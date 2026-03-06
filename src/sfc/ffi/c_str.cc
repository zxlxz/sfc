
#include "sfc/ffi/c_str.h"

namespace sfc::ffi {

auto CString::from_vec(Vec<char> v) -> CString {
  auto res = CString{};
  res._vec = mem::move(v);
  return res;
}

auto CString::from(Str s) -> CString {
  if (s.is_empty()) {
    return {};
  }

  auto vec = Vec<char>::with_capacity(s.len() + 1);
  vec.extend_from_slice({s._ptr, s._len});
  vec.push(0);
  return CString::from_vec(mem::move(vec));
}

auto CString::ptr() const -> const char* {
  return _vec.as_ptr();
}

auto CString::into_string() && -> String {
  auto vec = reinterpret_cast<Vec<u8>&&>(_vec);
  if (!vec.is_empty()) {
    vec.pop();
  }
  return String::from_utf8(mem::move(vec));
}

}  // namespace sfc::ffi
