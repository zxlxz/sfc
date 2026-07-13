#pragma once

#include "sfc/core/num.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

template <class T>
auto declval() -> T;

template <class F, class... A>
using FnOut = decltype(declval<F>()(declval<A>()...));

template <class>
struct Fn;

template <class R, class... T>
struct Fn<R(T...)> {
  class Self {};
  Self& _self;
  R (*_call)(Self&, T&&...);

 public:
  template <class X>
  static auto of(X& x) -> Fn {
    return {(Self&)x, [](Self& self, T&&... t) { return ((X&)self)((T&&)t...); }};
  }

 public:
  R operator()(T... t) const {
    return _call(_self, (T&&)t...);
  }
};

struct End {};
static constexpr auto $ = End{};

template <class T = usize>
struct Range {
  T _start;
  T _end;

 public:
  constexpr Range(T start, T end) : _start{start}, _end{end} {}
  constexpr Range(T start, End) : _start{start}, _end{num::Int<T>::MAX} {}

  constexpr auto wrap(T len) const noexcept -> Range {
    const auto s = _start < len ? _start : len;
    const auto e = _end < len ? _end : len;
    return {s, e};
  }

  constexpr auto len() const noexcept -> usize {
    return _start < _end ? usize{_end - _start} : 0UL;
  }

 public:
  // trait: cmp::Eq
  auto operator==(const Range& r) const noexcept -> bool {
    return _start == r._start && _end == r._end;
  }

  void fmt(auto& f) const {
    if (_end == num::Int<T>::MAX) {
      f.write_fmt("{}..$", _start);
    } else {
      f.write_fmt("{}..{}", _start, _end);
    }
  }

 public:
  // trait: iter::Iter
  auto operator*() const noexcept -> T {
    return _start;
  }

  // trait: iter::Iter
  void operator++() noexcept {
    ++_start;
  }
};

// trait: cmp::Eq(End)
template <class T>
auto operator!=(Range<T> self, End) noexcept -> bool {
  return self._start != self._end;
}

template <class T>
auto begin(Range<T> iter) -> Range<T> {
  return iter;
}

template <class T>
auto end(Range<T>) -> End {
  return {};
}

template <class T>
auto range(T start, T end) -> Range<T> {
  return {start, end};
}

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::range;
using ops::FnOut;
}  // namespace sfc
