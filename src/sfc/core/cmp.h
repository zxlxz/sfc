#pragma once

#include "sfc/core/trait.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace sfc::cmp {

template <class T>
constexpr auto min(T a, trait::arg_t<T> b) -> T {
  return a < b ? a : b;
}

template <class T>
constexpr auto max(T a, trait::arg_t<T> b) -> T {
  return a > b ? a : b;
}

}  // namespace sfc::cmp
