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

  auto as_view() -> Inn {
    return _inn;
  }

  auto as_data() const -> T* {
    return _inn.data();
  }

  auto as_mut_data() const -> T* {
    return _inn.data();
  }

  auto shape() const -> NdSize<N> {
    return _inn.shape();
  }

  auto strides() const -> NdStep<N> {
    return _inn.strides();
  }

  auto numel() const -> usize {
    return _inn.numel();
  }
};

}  // namespace sfc::math
