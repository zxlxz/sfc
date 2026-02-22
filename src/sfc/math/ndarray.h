#pragma once

#include "sfc/math/ndview.h"

namespace sfc::math {

template <class T, u32 N>
class NdArray {
  RawVec<T> _buf = {};
  NdView<T, N> _view = {};

 public:
  static auto with_shape(const u32 (&shape)[N]) -> NdArray {
    auto res = NdArray{};
    ptr::copy_nonoverlapping(shape, res._view._shape, N);
    res._buf = RawVec<T>::with_capacity(res._view.numel());
    res._view._data = res._buf.ptr();
    for (auto i = 0U; i < N; ++i) {
      res._view._strides[i] = i == 0 ? 1U : res._view._strides[i - 1] * shape[i - 1];
    }
    return res;
  }

  auto numel() const -> u32 {
    return _view.numel();
  }

  auto as_ptr() const noexcept -> const T* {
    return _buf.ptr();
  }

  auto as_mut_ptr() noexcept -> T* {
    return _buf.ptr();
  }

  auto as_view() -> NdView<T, N> {
    return _view;
  }
};

}  // namespace sfc::math
