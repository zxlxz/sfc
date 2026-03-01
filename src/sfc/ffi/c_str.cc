
#include "sfc/ffi/c_str.h"

namespace sfc::ffi {

auto CString::from(Str s) -> CString {
  auto res = CString{};
  res._vec.extend_from_slice({s._ptr, s._len});

  return res;
}

auto CString::into_string() && -> String {
  auto vec = reinterpret_cast<Vec<u8>&&>(_vec);
  if (!vec.is_empty()) {
    _vec.pop();
  }
  return String::from_utf8(mem::move(vec));
}

}  // namespace sfc::ffi
