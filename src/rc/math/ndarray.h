#pragma once

#include "rc/alloc/mod.h"
#include "rc/math/ndslice.h"

namespace rc::math {

template <class T, usize N>
struct NDArray {
  static constexpr usize RANK = N;
  using Dims = NDDims<RANK>;
  using Step = NDStep<RANK>;
  using Idxs = NDIdxs<RANK>;

  NDSlice<T, RANK> _inner;

  explicit NDArray(NDSlice<T, RANK> s) noexcept : _inner{s} {}

  ~NDArray() {
    if (_inner._data == nullptr) return;
    alloc::dealloc(_inner._data, _inner.count());
  }

  NDArray(NDArray&& other) noexcept : _inner{other._inner} {
    _inner._data = nullptr;
  }

  static auto with_dims(Dims dims) -> NDArray {
    const auto p = alloc::alloc<T>(dims.count());
    return NDArray{NDSlice<T, RANK>::from_array(p, dims)};
  }

  operator NDSlice<T, RANK>() const noexcept { return _inner; }

  auto as_ptr() const noexcept -> T* { return _inner._data; }
  auto as_mut_ptr() noexcept -> T* { return _inner._data; }

  auto as_slice() const noexcept -> NDSlice<T, RANK> { return _inner; }
  auto dims() const noexcept -> Dims { return _inner._dims; }
  auto count() const noexcept -> usize { return _inner.count(); }

  // op[call]: ref
  auto operator[](const Idxs& idxs) const noexcept -> const T& {
    return _inner[idxs];
  }

  // op[call]: ref
  auto operator[](const Idxs& idxs) noexcept -> T& { return _inner[idxs]; }

  template <usize... S, usize M = (... + (S - 1))>
  auto slice(const usize (&... idxs)[S]) const noexcept -> NDSlice<T, M> {
    return _inner.slice(idxs...);
  }

  // Send
  void operator<<(NDSlice<T, RANK> src) {}

  // Recv
  void operator>>(NDSlice<T, RANK> dst) {}
};

}  // namespace rc::math

namespace rc::fmt {

template <class T, usize N>
struct Display<math::NDArray<T, N>> {
  math::NDSlice<T, N> _self;

  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    fmt::Display<math::NDSlice<T, N>>{_self}.fmt(formatter);
  }
};

}  // namespace rc::fmt
