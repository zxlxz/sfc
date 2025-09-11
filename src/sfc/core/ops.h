#pragma once

#pragma once

#include "sfc/core/mod.h"

namespace sfc::ops {

struct UnderLine {};
static constexpr auto _ = UnderLine{};

struct Range {
  usize _start;
  usize _end;

 public:
  Range(usize start, usize end) : _start{start}, _end{end} {}

  Range(usize start, UnderLine) : _start{start}, _end{static_cast<usize>(-1)} {}

  auto wrap(usize len) const -> Range {
    const auto end = _end < len ? _end : len;
    return {_start, end};
  }

  auto len() const -> usize {
    return _start < _end ? _end - _start : 0U;
  }
};

}  // namespace sfc::ops
