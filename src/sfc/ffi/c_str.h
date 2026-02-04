#pragma once

#include "sfc/alloc/boxed.h"
#include "sfc/alloc/string.h"

namespace sfc::ffi {

class [[nodiscard]] CString {
  using Buf = Box<char[]>;
  Buf _buf = {};

 public:
  static auto xnew(Str s) noexcept -> CString {
    auto res = CString{};
    if (s._len != 0) {
      res._buf = Buf::xnew_uninit_slice(s._len + 1);
      ptr::copy_nonoverlapping(s._ptr, res._buf.ptr(), s._len);
      res._buf.ptr()[s._len] = 0;
    }
    return res;
  }

  auto as_ptr() const noexcept -> cstr_t {
    return _buf.ptr();
  }
};

}  // namespace sfc::ffi
