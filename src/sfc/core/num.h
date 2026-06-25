#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::int_ T>
struct Int;

template <trait::float_ T>
struct Float;

template <class T>
using uint_t = typename Int<T>::uint_t;

template <class T>
using sint_t = typename Int<T>::sint_t;

template <>
struct Int<unsigned char> {
  using uint_t = unsigned char;
  using sint_t = signed char;
  static constexpr uint_t MAX = 0xFF;
  static constexpr uint_t MIN = 0;
};

template <>
struct Int<unsigned short> {
  using uint_t = unsigned short;
  using sint_t = short;
  static constexpr uint_t MAX = 0xFFFF;
  static constexpr uint_t MIN = 0;
};

template <>
struct Int<unsigned int> {
  using uint_t = unsigned int;
  using sint_t = int;
  static constexpr uint_t MAX = 0xFFFFFFFF;
  static constexpr uint_t MIN = 0;
};

template <>
struct Int<unsigned long> {
  using uint_t = unsigned long;
  using sint_t = long;
#ifdef _WIN32
  static constexpr uint_t MAX = 0xFFFFFFFF;
#else
  static constexpr uint_t MAX = 0xFFFFFFFFFFFFFFFF;
#endif
  static constexpr uint_t MIN = 0;
};

template <>
struct Int<unsigned long long> {
  using uint_t = unsigned long long;
  using sint_t = long long;
  static constexpr uint_t MAX = 0xFFFFFFFFFFFFFFFF;
  static constexpr uint_t MIN = 0;
};

template <>
struct Int<signed char> {
  using uint_t = unsigned char;
  using sint_t = signed char;
  static constexpr sint_t MAX = +0x7F;
  static constexpr sint_t MIN = -0x80;
};

template <>
struct Int<short> {
  using uint_t = unsigned short;
  using sint_t = short;
  static constexpr sint_t MAX = +0x7FFF;
  static constexpr sint_t MIN = -0x8000;
};

template <>
struct Int<int> {
  using uint_t = unsigned int;
  using sint_t = int;
  static constexpr sint_t MAX = +0x7FFFFFFF;
  static constexpr sint_t MIN = -0x7FFFFFFF - 1;
};

template <>
struct Int<long> {
  using uint_t = unsigned long;
  using sint_t = long;
#ifdef _WIN32
  static constexpr sint_t MAX = +0x7FFFFFFF;
  static constexpr sint_t MIN = -MAX - 1;
#else
  static constexpr sint_t MAX = +0x7FFFFFFFFFFFFFFF;
  static constexpr sint_t MIN = -0x7FFFFFFFFFFFFFFF - 1;
#endif
};

template <>
struct Int<long long> {
  using uint_t = unsigned long long;
  using sint_t = long long;
  static constexpr sint_t MAX = +0x7FFFFFFFFFFFFFFF;
  static constexpr sint_t MIN = -0x7FFFFFFFFFFFFFFF - 1;
};

template <>
struct Float<float> {
  using float_t = float;
  using int_t = int;
};

template <>
struct Float<double> {
  using float_t = double;
  using int_t = long long;
};

template <trait::uint_ T>
constexpr auto cast_signed(T x) -> num::sint_t<T> {
  return __builtin_bit_cast(num::sint_t<T>, x);
}

template <trait::sint_ T>
constexpr auto cast_unsigned(T x) -> num::uint_t<T> {
  return __builtin_bit_cast(num::uint_t<T>, x);
}

template <trait::int_ T, trait::int_ F>
constexpr auto saturating_cast(F x) -> T {
  static constexpr auto kMAX = Int<T>::MAX;
  static constexpr auto kMin = Int<T>::MIN;

  if constexpr (trait::sint_<F>) {
    if constexpr (trait::uint_<T>) {
      if (x < 0) return 0U;
    } else if constexpr (Int<F>::MIN < kMin) {
      if (x < kMin) return kMin;
    }
  }

  if constexpr (Int<F>::MAX > kMAX) {
    if (x > kMAX) return kMAX;
  }
  return T(x);
}

template <trait::sint_ T>
constexpr auto unsigned_abs(T x) -> num::uint_t<T> {
  using U = num::uint_t<T>;
  if (x >= 0) return __builtin_bit_cast(U, x);
  return U{0} - __builtin_bit_cast(U, x);
}

template <trait::uint_ T>
constexpr auto saturating_add(T a, T b) -> T {
  using Int = num::Int<T>;
  return a > Int::MAX - b ? Int::MAX : a + b;
}

template <trait::uint_ T>
constexpr auto saturating_sub(T a, T b) -> T {
  return a < b ? 0U : a - b;
}

template <trait::uint_ T>
constexpr auto align_up(T val, T align) -> T {
  const auto mask = T{align - 1};
  return (val + mask) & ~mask;
}

template <trait::uint_ T>
constexpr auto is_power_of_two(T val) -> bool {
  return (val & (val - 1)) == 0;
}

template <trait::uint_ T>
constexpr auto next_power_of_two(T n) -> T {
  if (num::is_power_of_two(n)) return n;
  auto t = T{1};
  while (t < n)
    t <<= 1;
  return t;
}

template <trait::float_ T>
constexpr auto trunc_to_int(T x) -> num::Float<T>::int_t {
  if (sizeof(T) == sizeof(float)) {
    return i32(x);
  } else {
    return i64(x);
  }
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) noexcept -> bool;

}  // namespace sfc::num
