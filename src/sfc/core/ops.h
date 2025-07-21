#pragma once

#pragma once

#include "sfc/core/mod.h"

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
};

}  // namespace sfc::ops
