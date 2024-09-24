#pragma once

#include "mod.h"

namespace sfc::cmp {

enum Ordering {
  Equal = 0,
  Less = -1,
  Greater = 1,
};

struct Lt {
  [[sfc_inline]] auto operator()(const auto& a, const auto& b) const -> bool {
    return a < b;
  }
};

struct Gt {
  [[sfc_inline]] auto operator()(const auto& a, const auto& b) const -> bool {
    return a > b;
  }
};

[[sfc_inline]] auto cmp(const auto& a, const auto& b) -> Ordering {
  if (a == b) {
    return Ordering::Equal;
  }
  return a < b ? Ordering::Less : Ordering::Greater;
}

[[sfc_inline]] constexpr auto min(const auto& a, const auto& b) -> auto {
  return a < b ? a : b;
}

[[sfc_inline]] constexpr auto max(const auto& a, const auto& b) -> auto {
  return a < b ? b : a;
}

}  // namespace sfc::cmp

namespace sfc {
using cmp::Ordering;
}
