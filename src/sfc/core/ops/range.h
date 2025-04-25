#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/num.h"

namespace sfc::ops {

struct Range {
  usize _start;
  usize _end;

 public:
  explicit Range(usize end) : _start{0}, _end{end} {}

  Range(usize start, usize end) : _start{start}, _end{end < start ? start : end} {}

  Range(usize start, Dummy) : _start{start}, _end{static_cast<usize>(-1)} {}

  auto len() const -> usize {
    return _start < _end ? _end - _start : 0U;
  }

  auto operator%(usize len) const -> Range {
    const auto start = _start < len ? _start : len;
    const auto end = _end < len ? _end : len;
    return Range{start, end};
  }
};

}  // namespace sfc::ops

namespace sfc {
using ops::Range;
}
