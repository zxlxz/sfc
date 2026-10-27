#pragma once

#include "sfc/core/num.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::range {

struct End {};
static constexpr auto $ = End{};

template <class T>
struct Iter : iter::Iterator<T> {
  T _val;
  T _end;

 public:
  auto next() noexcept -> Option<T> {
    if (_val == _end) return {};
    return _val++;
  }

 public:
  auto operator==(iter::End) const noexcept -> bool {
    return _val >= _end;
  }

  auto operator*() const noexcept -> T {
    return _val;
  }

  auto operator++() noexcept -> T {
    return _val++;
  }
};

template <class T = usize>
struct Range {
  T _start;
  T _end;

 public:
  constexpr Range(T end) : _start{0}, _end{end} {}
  constexpr Range(T start, T end) : _start{start}, _end{end} {}
  constexpr Range(T start, End) : _start{start}, _end{num::Int<T>::MAX} {}

  constexpr auto wrap(T len) const noexcept -> Range {
    const auto s = _start < len ? _start : len;
    const auto e = _end < len ? _end : len;
    return {s, e};
  }

  constexpr auto len() const noexcept -> usize {
    return _start < _end ? usize(_end - _start) : 0UL;
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_end == num::Int<T>::MAX) {
      f.write_fmt("{}..$", _start);
    } else {
      f.write_fmt("{}..{}", _start, _end);
    }
  }

 public:
  friend auto begin(Range self) -> Iter<T> {
    return {{}, self._start, self._end};
  }

  friend auto end([[maybe_unused]] Range self) -> iter::End {
    return {};
  }
};

}  // namespace sfc::range

namespace sfc {
using range::$;
using range::Range;
}  // namespace sfc
