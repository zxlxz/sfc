#pragma once

#include "sfc/core/trait.h"

namespace sfc::num {

template <trait::int_ T>
struct Int;

template <trait::flt_ T>
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

using I8 = num::Int<i8>;
using I16 = num::Int<i16>;
using I32 = num::Int<i32>;
using I64 = num::Int<i64>;

using U8 = num::Int<u8>;
using U16 = num::Int<u16>;
using U32 = num::Int<u32>;
using U64 = num::Int<u64>;

template <trait::uint_ T>
constexpr auto cast_signed(T x) -> num::sint_t<T> {
  return __builtin_bit_cast(num::sint_t<T>, x);
}

template <trait::sint_ T>
constexpr auto cast_unsigned(T x) -> num::uint_t<T> {
  return __builtin_bit_cast(num::uint_t<T>, x);
}

template <trait::sint_ T>
constexpr auto unsigned_abs(T x) -> num::uint_t<T> {
  using U = num::uint_t<T>;
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
