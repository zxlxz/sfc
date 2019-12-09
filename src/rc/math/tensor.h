#pragma once

#include "rc/core.h"

namespace rc::math {

template <class T>
struct Tensor;

#pragma region scalar
#define impl_scalar(T)                                         \
  template <>                                                  \
  struct Tensor<T> {                                           \
    static constexpr usize RANK = 0;                           \
    T _self;                                                   \
    auto len() const noexcept -> usize { return 0; }           \
    auto operator[](usize) const -> const T& { return _self; } \
  }

impl_scalar(i8);
impl_scalar(i16);
impl_scalar(i32);
impl_scalar(i64);

impl_scalar(u8);
impl_scalar(u16);
impl_scalar(u32);
impl_scalar(u64);

impl_scalar(f32);
impl_scalar(f64);
#undef impl_scalar
#pragma endregion

#pragma region NDSlice
template <class T, usize N>
struct NDSlice;

template <class T, usize N>
struct NDArray;

template <class T, usize N>
struct Tensor<NDSlice<T, N>> {
  static constexpr usize RANK = N;
  NDSlice<T, RANK> _self;

  auto len() const noexcept -> usize { return _self._dims[RANK - 1]; }

  auto operator[](usize idx) const -> NDSlice<T, RANK - 1> {
    using R = NDSlice<T, RANK - 1>;
    using I = const_seq_t<RANK - 1>;
    const auto pos = idx * _self._step[RANK - 1];
    return R::from_idxs(_self._data + pos, _self._dims, _self._step, I{});
  }
};

template <class T>
struct Tensor<NDSlice<T, 1>> {
  static constexpr usize RANK = 1;
  NDSlice<T, RANK> _self;

  auto len() const noexcept -> usize { return _self._dims[RANK - 1]; }

  auto operator[](usize idx) const -> const T& {
    const auto pos = idx * _self._step[0];
    return _self._data[pos];
  }

  auto operator[](usize idx) -> T& {
    const auto pos = idx * _self._step[0];
    return _self._data[pos];
  }
};

template <class T, usize N>
struct Tensor<NDArray<T, N>> {
  static constexpr usize RANK = N;
  NDSlice<T, RANK> _self;

  auto len() const noexcept -> usize { return _self._dims[RANK - 1]; }

  auto operator[](usize idx) const -> NDSlice<T, RANK - 1> {
    using I = const_seq_t<RANK - 1>;
    return {_self._data, _self._dims, _self._step, I{}};
  }
};

template <class T>
struct Tensor<NDArray<T, 1>> {
  static constexpr usize RANK = 1;
  NDSlice<T, RANK> _self;

  auto len() const noexcept -> usize { return _self._dims[RANK - 1]; }

  auto operator[](usize idx) const -> const T& {
    const auto pos = idx * _self._step[0];
    return _self._data[pos];
  }

  auto operator[](usize idx) -> T& {
    const auto pos = idx * _self._step[0];
    return _self._data[pos];
  }
};

#pragma endregion

#pragma region Map

template <class X>
struct Map;

template <class F, class X0>
struct Map<F(X0)> {
  static constexpr usize RANK = Tensor<X0>::RANK;
  static_assert(RANK > 0);
  Tensor<X0> _0;
};

template <class F, class X0, class X1>
struct Map<F(X0, X1)> {
  static constexpr usize RANK = Tensor<X0>::RANK | Tensor<X1>::RANK;
  static_assert(RANK > 0);

  Tensor<X0> _0;
  Tensor<X1> _1;
};

template <class F, class X0>
struct Tensor<Map<F(X0)>> {
  using Self = Map<F(X0)>;
  static constexpr usize RANK = Self::RANK;

  Self _self;

  auto len() const noexcept -> usize { return _0.len(); }

  auto operator[](usize idx) const noexcept {
    using R0 = decltype(_self._0[idx]);
    if constexpr (RANK == 1) {
      return F{}(_self._0._val);
    } else {
      return Map<F(R0)>{_self._0[idx]};
    }
  }
};

template <class F, class X0, class X1>
struct Tensor<Map<F(X0, X1)>> {
  using Self = Map<F(X0, X1)>;
  static constexpr usize RANK = Self::RANK;

  Self _self;

  auto len() const noexcept -> usize { return _self._0.len() | _self._1.len(); }

  auto operator[](usize idx) const noexcept {
    using R0 = decltype(_self._0[idx]);
    using R1 = decltype(_self._1[idx]);
    if constexpr (RANK == 1) {
      return F{}(_self._0[idx], _self._1[idx]);
    } else {
      return Map<F(R0, R1)>{_self._0[idx], _self._1[idx]};
    }
  }
};
#pragma endregion

#pragma region Reduce
template <class X>
struct Reduce;

template <class F, class X0>
struct Reduce<F(X0)> {
  static constexpr usize RANK = Tensor<X0>::RANK - 1;
  static_assert(Tensor<X0>::RANK > 1);

  Tensor<X0> _0;
};

template <class F, class X0>
struct Tensor<Reduce<F(X0)>> {
  using Self = Reduce<F(X0)>;
  static constexpr usize RANK = Self::RANK;

  Self _self;

  auto len() const noexcept -> usize { return _self._0[0].len(); }

  auto operator[](usize idx) const noexcept {
    using R0 = decltype(_self._0[idx]);
    if constexpr (RANK == 1) {
      const auto n = _self._0.len();
      auto x = _self._0[0];
      for (usize i = 1; i < n; ++i) {
        x = F{}(_self._0[i]);
      }
      return x;
    } else {
      return Reduce<F(R0)>{_self._0[idx]};
    }
  }
};
#pragma endregion

#pragma region Linespace
template <class T, usize N>
struct Linspace {
  static constexpr usize RANK = N;
  T _step[RANK];

  template <class S, usize... I>
  static auto from_idxs(const S& step, Const<I...>) -> Linspace {
    return {step[I]...};
  }
};

template <class T, usize N>
Linspace(const T (&)[N])->Linspace<T, N>;

template <class T>
struct Tensor<Linspace<T, 1>> {
  static constexpr auto RANK = 1;
  using Item = T;

  Linspace<T, RANK> _self;

  auto len() const noexcept -> usize { return 0; }
  auto operator[](usize idx) const -> T { return T(idx * _self._step[0]); }
};



template <class T, usize N>
struct Tensor<Linspace<T, N>> {
  static constexpr auto RANK = N;
  using Item = Linspace<T, RANK - 1>;
  Linspace<T, RANK> _self;

  auto len() const noexcept -> usize { return 0; }

  auto operator[](usize idx) const -> Map<ops::Add(T, Linspace<T, RANK - 1>)> {
    const auto x = idx * _self._step[RANK - 1];
    const auto y = Item::from_idxs(_self._step, const_seq_t<RANK - 1>{});
    return {{x}, {y}};
  }
};
#pragma endregion

#pragma region Eye

#pragma endregion

#pragma region map : x?y
template <class A, class B>
constexpr auto _if_bop() -> usize {
  using TA = Tensor<A>;
  using TB = Tensor<B>;
  if constexpr (TA::RANK == 0 && TB::RANK == 0) {
    return false;
  }
  if constexpr (TA::RANK == 0 || TB::RANK == 0) {
    return true;
  }
  return TA::RANK == TB::RANK;
}

template <class T0, class T1>
using where_bop_t = rc::where_t<math::_if_bop<T0, T1>()>;

#define impl_bop(F, op)                                             \
  template <class A, class B, class = math::where_bop_t<A, B>>      \
  auto operator op(const A& a, const B& b) noexcept->Map<F(A, B)> { \
    return Map<F(A, B)>{{a}, {b}};                                  \
  }                                                                 \
  using rc::unit

impl_bop(ops::Add, +);
impl_bop(ops::Sub, -);
impl_bop(ops::Mul, *);
impl_bop(ops::Div, /);

impl_bop(ops::Eq, ==);
impl_bop(ops::Ne, !=);

impl_bop(ops::Gt, >);
impl_bop(ops::Le, <);
impl_bop(ops::Ge, >=);
impl_bop(ops::Le, <=);

#undef impl_bop
#pragma endregion

#pragma region invoke
template <class F, class LHS, class RHS>
void compute(LHS y, RHS x) {
  const auto n = y.len();
  if constexpr (LHS::RANK == 1) {
    for (usize i = 0; i < n; ++i) {
      F{}(y[i], x[i]);
    }
  } else {
    for (usize i = 0; i < n; ++i) {
      using YI = Tensor<decltype(y[i])>;
      using XI = Tensor<decltype(x[i])>;
      auto yi = YI{y[i]};
      auto xi = XI{x[i]};
      math::compute<F>(yi, xi);
    }
  }
}

template<class T, usize N, class U>
void operator<<=(NDSlice<T, N> lhs, const U& rhs) {
  auto y = Tensor<NDSlice<T, N>>{lhs};
  auto x = Tensor<U>{rhs};
  math::compute<ops::Assign>(y, x);
}

template <class T, usize N, class U>
void operator<<=(NDArray<T, N>& lhs, const U& rhs) {
  auto y = Tensor<NDSlice<T, N>>{lhs.as_slice()};
  auto x = Tensor<U>{rhs};
  math::compute<ops::Assign>(y, x);
}
#pragma endregion

}  // namespace rc::math
