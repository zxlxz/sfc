#pragma once

#include "sfc/core/mod.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

struct End {};

static constexpr auto $ = End{};

struct Range {
  usize start;
  usize end;

 public:
  Range(usize start, End) noexcept : start{start}, end{static_cast<usize>(-1)} {}
  Range(usize start, usize end) noexcept : start{start}, end{end} {}
};

auto add(auto val) {
  return [val](auto x) { return x + val; };
}

}  // namespace sfc::ops

namespace sfc {
using ops::$;
}
