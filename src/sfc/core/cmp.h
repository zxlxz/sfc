#pragma once

#include "mod.h"

namespace sfc::cmp {

enum Ordering {
  Equal = 0,
  Less = -1,
  Greater = 1,
};

struct Lt {
  auto operator()(const auto& a, const auto& b) const -> bool {
    return a < b;
  }
};

struct Gt {
  auto operator()(const auto& a, const auto& b) const -> bool {
    return a > b;
  }
};

auto cmp(const auto& a, const auto& b) -> Ordering {
  if (a == b) {
    return Ordering::Equal;
  }
  return a < b ? Ordering::Less : Ordering::Greater;
}

constexpr auto min(const auto& a, const auto& b) -> auto {
  return a < b ? a : b;
}

constexpr auto max(const auto& a, const auto& b) -> auto {
  return a < b ? b : a;
}

}  // namespace sfc::cmp

namespace sfc {
using cmp::Ordering;
}
