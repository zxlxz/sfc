#pragma once

#include "rc/core/mod.h"

namespace rc::ops {

#define impl_uop(Type, ...)                    \
  struct Type {                                \
    template <class T>                         \
    __forceinline auto operator()(T t) const { \
      return __VA_ARGS__;                      \
    }                                          \
  }

#define impl_bop(Type, ...)                         \
  struct Type {                                     \
    template <class T>                              \
    __forceinline auto operator()(T a, T b) const { \
      return __VA_ARGS__;                           \
    }                                               \
  }

#define impl_fn1(Type, op)                          \
  struct Type {                                     \
    template <class T>                              \
    __forceinline auto operator()(T t) const -> T { \
      if constexpr (rc::is_same<T, f32>()) {      \
        return ::__builtin_##op##f(t);              \
      }                                             \
      if constexpr (rc::is_same<T, f64>()) {      \
        return ::__builtin_##op(t);                 \
      }                                             \
    }                                               \
  }

#define impl_ass(Type, ...)                          \
  struct Type {                                      \
    template <class T>                               \
    __forceinline void operator()(T& y, T x) const { \
      __VA_ARGS__;                                   \
    }                                                \
  }

impl_uop(Pos, +t);
impl_uop(Neg, -t);

impl_bop(Eq, a == b);
impl_bop(Ne, a != b);
impl_bop(Lt, a < b);
impl_bop(Gt, a > b);
impl_bop(Le, a <= b);
impl_bop(Ge, a >= b);

impl_bop(Add, a + b);
impl_bop(Sub, a - b);
impl_bop(Mul, a* b);
impl_bop(Div, a / b);
impl_bop(Mod, a % b);

impl_bop(Max, a > b ? a : b);
impl_bop(Min, a < b ? a : b);

impl_fn1(Fabs, fabs);

impl_fn1(Sqrt, sqrt);
impl_fn1(Cbrt, cbrt);
impl_fn1(Exp, exp);
impl_fn1(Exp2, exp2);
impl_fn1(Log, log);
impl_fn1(Log2, log2);
impl_fn1(Log10, log10);
impl_fn1(Log1p, log1p);

impl_fn1(Sin, sin);
impl_fn1(Cos, cos);
impl_fn1(Tan, tan);

impl_fn1(ASin, asin);
impl_fn1(ACos, acos);
impl_fn1(ATan, atan);

impl_fn1(SinH, sinh);
impl_fn1(CosH, cosh);
impl_fn1(TanH, tanh);

impl_fn1(ASinH, asinh);
impl_fn1(ACosH, acosh);
impl_fn1(ATanH, atanh);

impl_ass(Assign, y = x);
impl_ass(AddAssign, y += x);
impl_ass(SubAssign, y -= x);
impl_ass(MulAssign, y *= x);
impl_ass(DivAssign, y /= x);
impl_ass(ModAssign, y %= x);

#undef impl_uop
#undef impl_bop
#undef impl_fn1
#undef impl_ass

template <usize N>
constexpr auto any(const bool (&v)[N]) -> bool {
  auto res = v[0];
  if constexpr (N > 1) {
    for (usize i = 1; i < N; ++i) {
      res |= v[i];
    }
  }
  return res;
}

template <usize N>
constexpr auto all(const bool(&v)[N]) -> bool {
  auto res = v[0];
  if constexpr (N > 1) {
    for (usize i = 1; i < N; ++i) {
      res &= v[i];
    }
  }
  return res;
}

template <class T, usize N>
constexpr auto sum(const T (&v)[N]) -> T {
  auto res = v[0];
  if constexpr (N > 1) {
    for (usize i = 1; i < N; ++i) {
      res += v[i];
    }
  }
  return res;
}

template<class T, usize N>
constexpr auto prod(const T(&v)[N]) -> T{
  auto res = v[0];
  if constexpr (N > 1) {
    for (usize i = 1; i < N; ++i) {
      res *= v[i];
    }
  }
  return res;
}

template <usize N>
constexpr auto count(const bool (&v)[N]) -> usize {
  auto res = usize(0);
  if constexpr (N > 1) {
    for (usize i = 0; i < N; ++i) {
      if (v[i]) res += 1;
    }
  }
  return res;
}

#ifdef max
#undef max
#endif
template <class T, class... U>
constexpr auto max(const T& arg0, const U&... args) {
  if constexpr (sizeof...(U) == 0) {
    return arg0;
  } else if constexpr (sizeof...(U) == 1) {
    return ops::Max{}(arg0, args...);
  } else {
    return ops::Max{}(arg0, ops::max(args...));
  }
}

#ifdef min
#undef min
#endif
template <class T, class... U>
constexpr auto min(const T& arg0, const U&... args) {
  if constexpr (sizeof...(U) == 0) {
    return arg0;
  } else if constexpr (sizeof...(U) == 1) {
    return ops::Min{}(arg0, args...);
  } else {
    return ops::Min{}(arg0, ops::min(args...));
  }
}

template <class X>
struct Invoke;

template <class F, class... Args>
struct Invoke<F(Args...)> {
  using Output = decltype(rc::declval<F>()(rc::declval<Args>()...));
};

template <class X>
using invoke_t = typename Invoke<X>::Output;

}  // namespace rc::ops
