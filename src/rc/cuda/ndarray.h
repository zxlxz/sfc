#pragma once

#include "rc/cuda/mod.h"
#include "rc/math/ndslice.h"

namespace rc::cuda {

using math::NDSlice;

template <class T, usize N>
struct NDArray {
  static constexpr usize RANK = N;
  using Dims = math::NDDims<RANK>;
  using Step = math::NDStep<RANK>;
  using Idxs = math::NDIdxs<RANK>;

  NDSlice<T, RANK> _inner;

  explicit NDArray(NDSlice<T, RANK> s) noexcept : _inner{s} {}

  ~NDArray() {
    const auto p = _inner._data;
    const auto n = _inner.count();
    if (p == nullptr) return;
    Alloc::dealloc(p, alloc::Layout::array<T>(n));
  }

  NDArray(NDArray&& other) noexcept : _inner{other._inner} {
    _inner._data = nullptr;
  }

  static auto with_dims(Dims dims, MemType type = MemType::Device) -> NDArray {
    const auto n = dims.count();
    const auto p = Alloc::alloc(alloc::Layout::array<T>(n), type);
    const auto s = NDSlice<T, RANK>::from_array(ptr::cast<T>(p), dims);
    return NDArray{s};
  }
};

}  // namespace rc::cuda

