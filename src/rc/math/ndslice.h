#pragma once

#include "rc/math/mod.h"

namespace rc::math {

#pragma region NDSlice
template <class T, usize N>
struct NDSlice {
  constexpr static usize RANK = N;
  using Item = T;
  using Dims = NDDims<RANK>;
  using Step = NDStep<RANK>;
  using Idxs = NDIdxs<RANK>;

  T*   _data;
  Dims _dims;
  Step _step;

  constexpr NDSlice(T* data, const Dims& dims, const Step& step) noexcept
      : _data{data}, _dims{dims}, _step{step} {}


  static auto from_array(T* data, const Dims& dims) -> NDSlice {
    const auto step = Step::from_dims(dims);
    return NDSlice { data, dims, step };
  }

  template <class D, class S, usize... I>
  static auto from_idxs(T* p, const D& d, const S& s, Const<I...>) -> NDSlice {
    return NDSlice{p, {d[I]...}, {s[I]...}};
  }

  __forceinline auto as_ptr() const noexcept -> const T* { return _data; }

  __forceinline auto as_mut_ptr() noexcept -> T* { return _data; }

  __forceinline auto dims() const noexcept -> Dims {
    return _dims;
  }

  __forceinline auto step() const noexcept -> Step {
    return _step;
  }

  __forceinline auto count() const noexcept -> usize {
    return _dims.count();
  }

  auto is_array() const noexcept -> bool {
    return _step == Step::from_dims(_dims);
  }

  // op[call]: ref
  __forceinline auto operator[](const Idxs& idxs) const noexcept -> const T& {
    return _data[idxs^_step];
  }

  // op[call]: ref
  __forceinline auto operator[](const Idxs& idxs) noexcept -> T& {
    return _data[idxs^_step];
  }

  template <usize... S, usize M = (...+(S-1))>
  auto slice(const usize (&... idxs)[S]) const noexcept -> NDSlice<T, M> {
    using I = const_find_t<(S != 1)...>;
    const Range ss[] = {Range::from(idxs)...};

    usize pos = 0;
    Dims dims = {0};
    for (usize i = 0; i < RANK; ++i) {
      const auto s = Range(ss[i]).wrap(_dims[i]);
      pos += s.idx() * _step[i];
      dims._0[i] = s.len();
    }

    const auto data = this->_data + pos;
    return NDSlice<T, M>::from_idxs(data, dims, _step, I{});
  }
};

template <class T, usize N>
NDSlice(T*, NDDims<N>, NDStep<N>)->NDSlice<T, N>;
#pragma endregion

}  // namespace rc::math

namespace rc::fmt {

template <class T>
struct Display<math::NDSlice<T, 1>> {
  math::NDSlice<T, 1> _self;

  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter._depth += 1;
    for (usize i0 = 0; i0 < _self._dims[0]; ++i0) {
      formatter.write_str(u8"\n");
      formatter.write_val(_self[{i0}]);
    }
    formatter._depth -= 1;
  }
};

template <class T>
struct Display<math::NDSlice<T, 2>> {
  math::NDSlice<T, 2> _self;

  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter._depth += 1;
    for (usize i0 = 0; i0 < _self._dims[0]; ++i0) {
      formatter.write_str(u8"\n");
      const auto rows = _self.slice({i0}, math::_);
      for (usize i1 = 0; i1 < _self._dims[1]; ++i1) {
        formatter.write_val(rows[{i1}]);
        formatter.write_str(u8", ");
      }
    }
    formatter._depth -= 1;
  }
};

}  // namespace rc::fmt
