#pragma once

#include "rc/cuda/mod.h"
#include "rc/math.h"

namespace rc::cuda {

using math::NDSlice;

template <class T, usize N>
struct NDArray {
  static constexpr usize RANK = N;
  using Self = NDArray;

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

  auto as_ptr() const noexcept -> T* { return _inner._data; }
  auto as_ptr_mut() noexcept -> T* { return _inner._data; }

  auto as_slice() const noexcept -> NDSlice<T, RANK> { return _inner; }
  auto dims() const noexcept -> Dims { return _inner._dims; }
  auto count() const noexcept -> usize { return _inner.count(); }


  void operator<<=(math::NDSlice<T, N> src) const {
    rc::assert(src.is_array(), u8"rc::cuda: NDSlice not array");
    rc::assert(src.dims() == this->dims(), u8"rc::cuda: dims not match");
    cuda::copy(src.as_ptr(), this->as_ptr(), src.count());
  }

  void operator>>=(math::NDSlice<T, N> dst) {
    rc::assert(dst.is_array(), u8"rc::cuda: NDSlice not array");
    rc::assert(dst.dims() == this->dims(), u8"rc::cuda: dims not match");
    cuda::copy(this->as_ptr(), dst.as_mut_ptr(), dst.count());
  }

};

}  // namespace rc::cuda
