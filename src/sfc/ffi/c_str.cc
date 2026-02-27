
#include "sfc/ffi/c_str.h"

namespace sfc::ffi {

auto CString::from(Str s) -> CString {
  auto res = CString{};
  res._vec.extend_from_slice({s._ptr, s._len});

  return res;
}

auto CString::into_string() && -> String {
  static_assert(sizeof(CString) == sizeof(String));

  auto res = reinterpret_cast<String&&>(*this);
  return res;
}

}  // namespace sfc::ffi
