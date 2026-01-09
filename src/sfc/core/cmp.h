#pragma once

#include "sfc/core/mod.h"

namespace sfc::cmp {

template <class T>
auto min(T a, T b) -> T {
  return a < b ? a : b;
}

template <class T>
auto max(T a, T b) -> T {
  return a > b ? a : b;
}

}  // namespace sfc::cmp
