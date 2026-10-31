#pragma once

#include "sfc/core.h"

namespace sfc::io {

template <trait::tv_copy_ T>
auto bytes_of(const T& val) noexcept -> Slice<const u8> {
  const auto p = ptr::cast<const u8>(&val);
  const auto n = sizeof(val);
  return Slice{p, n};
}

template <trait::tv_copy_ T>
auto bytes_of_mut(T& val) noexcept -> Slice<u8> {
  const auto p = ptr::cast<u8>(&val);
  const auto n = sizeof(val);
  return Slice{p, n};
}

template <trait::tv_copy_ T>
auto cast_bytes(Slice<const T> v) noexcept -> Slice<const u8> {
  const auto p = ptr::cast<const u8>(v._ptr);
  const auto n = v._len * sizeof(T);
  return Slice{p, n};
}

template <trait::tv_copy_ T>
auto cast_bytes_mut(Slice<T> v) noexcept -> Slice<u8> {
  const auto p = ptr::cast<u8>(v._ptr);
  const auto n = v._len * sizeof(T);
  return Slice{p, n};
}

}  // namespace sfc::io
