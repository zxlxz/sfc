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

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
using ops::FnOut;
}  // namespace sfc
