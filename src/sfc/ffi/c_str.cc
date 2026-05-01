
#include "sfc/ffi/c_str.h"

namespace sfc::ffi {

auto CString::from(Str s) -> CString {
  if (s.is_empty()) {
    return {};
  }

  auto res = CString{};
  auto& buf = res._buf;
  buf.reserve(s.len() + 1);
  buf.extend_from_slice({s._ptr, s._len});
  buf.push(0);
  return res;
}

auto CString::ptr() const -> const char* {
  return _buf.as_ptr();
}

auto CString::into_string() && -> String {
  auto list = reinterpret_cast<List<u8>&&>(_buf);
  if (!list.is_empty()) {
    list.pop();
  }
  return String::from_buf(mem::move(list));
}

}  // namespace sfc::ffi
