#pragma once

#include "sfc/core/num.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

template <class T>
auto declval() -> T;

template <class T, T... I>
struct IntSeq {
  static auto eq(const auto& a, const auto& b) -> bool {
    return ((a[I] == b[I]) && ...);
  }

  static auto ne(const auto& a, const auto& b) -> bool {
    return ((a[I] != b[I]) || ...);
  }

  static auto lt(const auto& a, const auto& b) -> bool {
    return ((a[I] < b[I]) && ...);
  }

  static auto le(const auto& a, const auto& b) -> bool {
    return ((a[I] <= b[I]) && ...);
  }

  static auto gt(const auto& a, const auto& b) -> bool {
    return ((a[I] > b[I]) && ...);
  }

  static auto ge(const auto& a, const auto& b) -> bool {
    return ((a[I] >= b[I]) && ...);
  }

  static auto all(const auto& a) -> bool {
    return (a[I] && ...);
  }

  static auto any(const auto& a) -> bool {
    return (a[I] || ...);
  }

  static auto accum(const auto& a) {
    return (a[I] + ...);
  }

  static auto prod(const auto& a) {
    return (a[I] * ...);
  }

  static auto dot(const auto& a, const auto& b) {
    return ((a[I] * b[I]) + ...);
  }
};

template <u32... I>
using IdxSeq = IntSeq<u32, I...>;

#if __has_builtin(__make_integer_seq)
template <u32 N>
using index_seq = __make_integer_seq<IntSeq, u32, N>;
#else
template <u32 N>
using index_seq = IntSeq<u32, __integer_pack(N)...>;
#endif

template <class>
struct Fn;

template <class R, class... T>
struct Fn<R(T...)> {
  class Self {};
  using Func = R (*)(Self&, T&&...);

  Self& _self;
  Func _func;

 public:
  template <class X>
  explicit Fn(X& x, Func f) : _self{(Self&)x}, _func{f} {}

  template <class X>
  explicit Fn(X& x) : _self{(Self&)x} {
    _func = [](Self& self, T&&... t) -> R { return ((X&)self)((T&&)t...); };
  }

 public:
  R operator()(T... t) const {
    return _func(_self, (T&&)t...);
  }
};

template <auto f>
auto fn(auto& x) {
  auto conv = []<class X, class R, class... T>(const X& x, R (X::*)(T...)) {
    return Fn<R(T...)>{x, [](X& self, T... t) -> R { return ((X&)self.*f)((T&&)t...); }};
  };
  return conv(x, f);
}

template <class F, class... A>
using FnOut = decltype(declval<F>()(declval<A>()...));

struct End {};
static constexpr auto $ = End{};

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
    return _start < _end ? usize{_end - _start} : 0UL;
  }

 public:
  // trait: cmp::Eq
  auto operator==(const Range& r) const noexcept -> bool {
    return _start == r._start && _end == r._end;
  }

  // trait: cmp::Eq
  auto operator==(End) const noexcept -> bool {
    return _start == _end;
  }

  // trait: iter::Iter
  auto operator*() const noexcept -> T {
    return _start;
  }

  // trait: iter::Iter
  void operator++() noexcept {
    ++_start;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_end == num::Int<T>::MAX) {
      f.write_fmt("{}..$", _start);
    } else {
      f.write_fmt("{}..{}", _start, _end);
    }
  }
};

template <class T>
auto begin(Range<T> iter) -> Range<T> {
  return iter;
}

template <class T>
auto end(Range<T>) -> End {
  return {};
}

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::FnOut;
}  // namespace sfc
