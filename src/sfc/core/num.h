#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::int_ T>
struct Int;

template <trait::flt_ T>
struct Float;

template <>
struct Int<unsigned char> {
  using UInt = unsigned char;
  using SInt = signed char;
  static constexpr UInt MAX = 0xFF;
  static constexpr UInt MIN = 0;
};

template <>
struct Int<unsigned short> {
  using UInt = unsigned short;
  using SInt = short;
  static constexpr UInt MAX = 0xFFFF;
  static constexpr UInt MIN = 0;
};

template <>
struct Int<unsigned int> {
  using UInt = unsigned int;
  using SInt = int;
  static constexpr UInt MAX = 0xFFFFFFFF;
  static constexpr UInt MIN = 0;
};

template <>
struct Int<unsigned long> {
  using UInt = unsigned long;
  using SInt = long;
#ifdef _WIN32
  static constexpr UInt MAX = 0xFFFFFFFF;
#else
  static constexpr UInt MAX = 0xFFFFFFFFFFFFFFFF;
#endif
  static constexpr UInt MIN = 0;
};

template <>
struct Int<unsigned long long> {
  using UInt = unsigned long long;
  using SInt = long long;
  static constexpr UInt MAX = 0xFFFFFFFFFFFFFFFF;
  static constexpr UInt MIN = 0;
};

template <>
struct Int<signed char> {
  using UInt = unsigned char;
  using SInt = signed char;
  static constexpr SInt MAX = +0x7F;
  static constexpr SInt MIN = -0x80;
};

template <>
struct Int<short> {
  using UInt = unsigned short;
  using SInt = short;
  static constexpr SInt MAX = +0x7FFF;
  static constexpr SInt MIN = -0x8000;
};

template <>
struct Int<int> {
  using UInt = unsigned int;
  using SInt = int;
  static constexpr SInt MAX = +0x7FFFFFFF;
  static constexpr SInt MIN = -0x7FFFFFFF - 1;
};

template <>
struct Int<long> {
  using UInt = unsigned long;
  using SInt = long;
#ifdef _WIN32
  static constexpr SInt MAX = +0x7FFFFFFF;
  static constexpr SInt MIN = -MAX - 1;
#else
  static constexpr SInt MAX = +0x7FFFFFFFFFFFFFFF;
  static constexpr SInt MIN = -0x7FFFFFFFFFFFFFFF - 1;
#endif
};

template <>
struct Int<long long> {
  using UInt = unsigned long long;
  using SInt = long long;
  static constexpr SInt MAX = +0x7FFFFFFFFFFFFFFF;
  static constexpr SInt MIN = -0x7FFFFFFFFFFFFFFF - 1;
};

using I8 = num::Int<i8>;
using I16 = num::Int<i16>;
using I32 = num::Int<i32>;
using I64 = num::Int<i64>;

using U8 = num::Int<u8>;
using U16 = num::Int<u16>;
using U32 = num::Int<u32>;
using U64 = num::Int<u64>;

template <trait::uint_ T, class S = num::Int<T>::SInt>
constexpr auto cast_signed(T x) -> S {
  return __builtin_bit_cast(S, x);
}

template <trait::sint_ T, class U = num::Int<T>::UInt>
constexpr auto cast_unsigned(T x) -> U {
  return __builtin_bit_cast(U, x);
}

template <trait::sint_ T, class U = num::Int<T>::UInt>
constexpr auto unsigned_abs(T x) -> U {
  if (x >= 0) return __builtin_bit_cast(U, x);
  return U{0} - __builtin_bit_cast(U, x);
}

template <trait::int_ T>
constexpr auto saturating_add(T a, T b) -> T {
  auto c = T{0};
  const auto x = __builtin_add_overflow(a, b, &c);
  return x ? num::Int<T>::MAX : c;
}

template <trait::int_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  auto c = T{0};
  const auto x = __builtin_sub_overflow(a, b, &c);
  return x ? num::Int<T>::MIN : c;
}

template <trait::uint_ T>
constexpr auto align_up(T val, T align) -> T {
  const auto mask = T(align - T{1});
  return T((val + mask) & ~mask);
}

template <trait::flt_ T>
auto fabs(T val) -> T {
  if constexpr (trait::same_<T, f64>) return __builtin_fabs(val);
  if constexpr (trait::same_<T, f32>) return __builtin_fabsf(val);
}

template <trait::flt_ T>
auto ceil(T val) -> T {
  if constexpr (trait::same_<T, f64>) return __builtin_ceil(val);
  if constexpr (trait::same_<T, f32>) return __builtin_ceilf(val);
}

template <trait::flt_ T>
auto floor(T val) -> T {
  if constexpr (trait::same_<T, f64>) return __builtin_floor(val);
  if constexpr (trait::same_<T, f32>) return __builtin_floorf(val);
}

template <trait::flt_ T>
auto fmin(T a, T b) -> T {
  if constexpr (trait::same_<T, f64>) return __builtin_fmin(a, b);
  if constexpr (trait::same_<T, f32>) return __builtin_fminf(a, b);
}

template <trait::flt_ T>
auto fmax(T a, T b) -> T {
  if constexpr (trait::same_<T, f64>) return __builtin_fmax(a, b);
  if constexpr (trait::same_<T, f32>) return __builtin_fmaxf(a, b);
}

}  // namespace sfc::num

namespace sfc {
using num::Int;

using num::U8;
using num::U16;
using num::U32;
using num::U64;

using num::I8;
using num::I16;
using num::I32;
using num::I64;
}  // namespace sfc
