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

struct End {
  consteval operator usize() const noexcept {
    return static_cast<usize>(-1);
  }
};

static constexpr auto $ = End{};

struct Range {
  usize start = 0;
  usize end = usize{$};
};

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
}  // namespace sfc
