#pragma once

#include "sfc/math/ndview.h"

namespace sfc::math {

template <class T, usize N>
class NdArray {
  using Inn = NdView<T, N>;
  using Buf = vec::Buf<T>;

  using Size = typename Inn::Size;
  using Step = typename Inn::Step;
  using Idxs = typename Inn::Idxs;

  Inn _inn;
  Buf _buf;

  NdArray(Inn inn, Buf buf) noexcept : _inn{inn}, _buf{mem::move(buf)} {}

 public:
  NdArray() = default;

  ~NdArray() = default;

  NdArray(NdArray&& other) noexcept = default;

  NdArray& operator=(NdArray&& other) noexcept = default;

  [[sfc_inline]] auto as_view() -> Inn {
    return _inn;
  }

  [[sfc_inline]] auto as_data() const -> T* {
    return _inn.data();
  }

  [[sfc_inline]] auto as_mut_data() const -> T* {
    return _inn.data();
  }

  [[sfc_inline]] auto shape() const -> NdSize<N> {
    return _inn.shape();
  }

  [[sfc_inline]] auto strides() const -> NdStep<N> {
    return _inn.strides();
  }

  [[sfc_inline]] auto numel() const -> usize {
    return _inn.numel();
  }
};

}  // namespace sfc::math
