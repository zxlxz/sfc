#pragma once

#include "sfc/core/mod.h"

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

template <class Self, class>
struct DynFn;

struct End {};
static constexpr auto $ = End{};

struct Range {
  usize start;
  usize end;

 public:
  constexpr Range(usize start, usize end) : start{start}, end{end} {}
  constexpr Range(usize start, End) : start{start}, end{usize(-1)} {}

  constexpr auto wrap(usize len) const noexcept -> Range {
    const auto s = start < len ? start : len;
    const auto e = end < len ? end : len;
    return {s, e};
  }

  constexpr auto len() const noexcept -> usize {
    return start < end ? end - start : 0UL;
  }
};

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
using ops::FnOut;
}  // namespace sfc
