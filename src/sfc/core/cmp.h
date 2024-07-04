#pragma once

#include "mod.h"

namespace sfc::cmp {

enum Ordering {
  Equal = 0,
  Less = -1,
  Greater = 1,
};

struct Lt {
  template <class T>
  [[sfc_inline]] auto operator()(const T& a, const T& b) const -> bool {
    return a < b;
  }
};

struct Gt {
  template <class T>
  [[sfc_inline]] auto operator()(const T& a, const T& b) const -> bool {
    return !(a < b);
  }
};

template <class T>
[[sfc_inline]] auto cmp(const T& a, const T& b) -> Ordering {
  if (a == b) return Ordering::Equal;
  return a < b ? Ordering::Less : Ordering::Greater;
}

template <class T>
[[sfc_inline]] constexpr auto min(const T& a, const T& b) -> const T& {
  return a < b ? a : b;
}

template <class T>
[[sfc_inline]] constexpr auto max(const T& a, const T& b) -> const T& {
  return a < b ? b : a;
}

}  // namespace sfc::cmp

namespace sfc {
using cmp::Ordering;
}
