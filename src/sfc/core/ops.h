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
  Range(usize idx, End) noexcept : start{idx}, end{static_cast<usize>(-1)} {}

  Range(usize start, usize end) noexcept : start{start}, end{end} {}
};

}  // namespace sfc::ops

namespace sfc {
using ops::$;
}
