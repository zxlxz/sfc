#pragma once

#include "rc/core/str.h"

namespace rc::fmt {

struct Buffer {
  u8* _ptr;
  usize _cap;
  usize _len;

  template <usize N>
  static auto from_array(u8 (&v)[N]) -> Buffer {
    return Buffer{ptr::cast<u8>(v), N, 0};
  }

  static auto from_slice(Slice<u8> v) -> Buffer {
    return Buffer{v._ptr, v._len, 0};
  }

  auto capacity() const noexcept -> usize { return _cap; }

  auto len() const noexcept -> usize { return _len; }

  auto rem() const noexcept -> usize { return _cap - _len; }

  auto as_bytes() const -> Slice<const u8> { return {_ptr, _len}; }

  auto as_str() const noexcept -> Str { return Str{_ptr, _len}; }

  auto clear() noexcept -> void { _len = 0; }

  auto write(Slice<const u8> buf) noexcept -> usize {
    if (buf.is_empty()) return 0u;

    const auto cnt = ops::min(buf._len, this->rem());
    ptr::copy(buf._ptr, _ptr + _len, cnt);
    this->_len += cnt;

    return cnt;
  }
};

}  // namespace rc::fmt
