#pragma once

#include "sfc/core/mod.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

template <class T>
auto declval() -> T&&;

template <class X>
struct Invoke;

template <class F, class... T>
struct Invoke<F(T...)> {
  using Output = decltype(declval<F>()(declval<T>()...));
};

template <class X>
using invoke_t = Invoke<X>::Output;

struct End {};

static constexpr auto $ = End{};

struct Range {
  usize start;
  usize end;

  Range(usize start, usize end) : start{start}, end{end} {}
  Range(usize start, End) : start{start}, end{usize(-1)} {}
};

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
}  // namespace sfc
