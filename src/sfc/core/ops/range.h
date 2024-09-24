#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/num.h"

namespace sfc::ops {

template <class T = usize>
struct Range {
  T _start;
  T _end;

 public:
  [[sfc_inline]] Range(T end) : _start{0}, _end{end} {}

  [[sfc_inline]] Range(T start, T end) : _start{start}, _end{end} {}

  [[sfc_inline]] Range(usize start, Dummy) : _start{start}, _end{num::max_value<usize>()} {}

  [[sfc_inline]] auto len() const -> usize {
    return _start < _end ? static_cast<usize>(_end - _start) : 0U;
  }

  [[sfc_inline]] auto operator%(usize len) const -> Range {
    const auto start = _start < len ? _start : len;
    const auto end = _end < len ? _end : len;
    return Range{start, end};
  }
};

template <class T>
Range(T) -> Range<T>;

template <class T>
Range(T, T) -> Range<T>;

}  // namespace sfc::ops

namespace sfc {
using ops::Range;
}
