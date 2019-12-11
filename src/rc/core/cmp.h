#pragma once

#include "rc/core/ptr.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace rc::cmp {

template <class T>
constexpr auto max(const T& a, const T& b) -> T {
  return a > b ? a : b;
}

template <class T>
constexpr auto min(const T& a, const T& b) -> T {
  return a < b ? a : b;
}

template <class T>
auto all_eq(const T* a, const T* b, usize n) -> bool {
  for (const T* e = a + n; a != e; ++a, ++b) {
    if (!(*a == *b)) {
      return false;
    }
  }
  return true;
}

template <class T>
auto any_ne(const T* a, const T* b, usize n) -> bool {
  for (const T* e = a + n; a != e; ++a, ++b) {
    if (!(*a == *b)) {
      return true;
    }
  }
  return false;
}

template <class T>
auto partial_cmp(const T* a, const T* b, usize n) -> isize {
  for (const T* e = a + n; a < e; ++a, ++b) {
    if (*a != *b) {
      const auto x = a - (e - n);
      return *a < *b ? -isize(x) : +isize(x);
    }
  }
  return 0;
}

template <class T, class U>
auto operator==(const T& lhs, const U& rhs) -> bool {
  return lhs.eq(rhs);
}

template <class T, class U>
auto operator!=(const T& lhs, const U& rhs) -> bool {
  return !lhs.eq(rhs);
}

template <class T>
auto operator<(const T& lhs, const T& rhs) -> bool {
  return lhs.partial_cmp(rhs) < 0;
}

template <class T>
auto operator>(const T& lhs, const T& rhs) -> bool {
  return lhs.partial_cmp(rhs) < 0;
}

template <class T>
auto operator<=(const T& lhs, const T& rhs) -> bool {
  return lhs.partial_cmp(rhs) <= 0;
}

template <class T>
auto operator>=(const T& lhs, const T& rhs) -> bool {
  return lhs.partial_cmp(rhs) >= 0;
}

}  // namespace rc::cmp

namespace rc {
using cmp::operator==;
using cmp::operator!=;

using cmp::operator>;
using cmp::operator<;
using cmp::operator>=;
using cmp::operator<=;
}  // namespace rc
