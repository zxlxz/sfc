#pragma once

#include "sfc/core/mod.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

template <class T>
auto declval() -> T&&;

template <class F, class... A>
using FnOut = decltype(declval<F>()(declval<A>()...));

struct End {};
static constexpr auto $ = End{};

struct Range {
  usize start;
  usize end;

 public:
  [[gnu::always_inline]] constexpr Range(usize start, usize end) : start{start}, end{end} {}
  [[gnu::always_inline]] constexpr Range(usize start, End) : start{start}, end{usize(-1)} {}

  [[gnu::always_inline]] constexpr auto wrap(usize len) const noexcept -> Range {
    const auto s = start < len ? start : len;
    const auto e = end < len ? end : len;
    return {s, e};
  }

  [[gnu::always_inline]] constexpr auto len() const noexcept -> usize {
    return start < end ? end - start : 0UL;
  }
};

template <auto X>
struct ConstVal {
  using Type = decltype(X);
  static constexpr auto VALUE = X;

  operator Type() const {
    return VALUE;
  }
};

template <usize N>
constexpr auto fold_seq(auto&& f) {
  static_assert(N > 0 && N <= 8, "assert (0<=N<=8) failed");

#define X(n) ConstVal<n>()
  if constexpr (N == 0) return;
  if constexpr (N == 1) return f(X(0));
  if constexpr (N == 2) return f(X(0), X(1));
  if constexpr (N == 3) return f(X(0), X(1), X(2));
  if constexpr (N == 4) return f(X(0), X(1), X(2), X(3));
  if constexpr (N == 5) return f(X(0), X(1), X(2), X(3), X(4));
  if constexpr (N == 6) return f(X(0), X(1), X(2), X(3), X(4), X(5));
  if constexpr (N == 7) return f(X(0), X(1), X(2), X(3), X(4), X(5), X(6));
  if constexpr (N == 8) return f(X(0), X(1), X(2), X(3), X(4), X(5), X(6), X(7));
#undef X
}

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
using ops::FnOut;
}  // namespace sfc
