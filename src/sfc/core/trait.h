#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

#if defined(_MSC_VER) && !defined(__clang__)
template <class T, class U>
struct IsSame {
  static constexpr bool VALUE = false;
};

template <class T>
struct IsSame<T, T> {
  static constexpr bool VALUE = true;
};
template <class T, class U>
concept same_ = IsSame<T, U>::VALUE;
#else
template <class T, class U>
concept same_ = __is_same(T, U);
#endif

template <class T, class... U>
concept any_ = (... || trait::same_<T, U>);

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

template <class T>
concept float_ = any_<T, float, double>;

template <class T>
concept signed_ = any_<T, signed char, short, int, long, long long>;

template <class T>
concept unsigned_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = signed_<T> || unsigned_<T>;

}  // namespace sfc::trait
