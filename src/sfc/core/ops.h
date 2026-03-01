#pragma once

#include "sfc/core/trait.h"

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
  template <trait::uint_ T>
  consteval operator T() const noexcept {
    return static_cast<T>(-1);
  }
};

static constexpr auto $ = End{};

struct Range {
  usize start = 0;
  usize end = usize{$};
};

auto eq(const auto& a, const auto& b) -> bool {
  using slice::Slice;
  if constexpr (requires { Slice{a} == Slice{b}; }) {
    return Slice{a} == Slice{b};
  } else {
    return a == b;
  }
}

auto ne(const auto& a, const auto& b) -> bool {
  using slice::Slice;
  if constexpr (requires { Slice{a} == Slice{b}; }) {
    return Slice{a} != Slice{b};
  } else {
    return a != b;
  }
}

}  // namespace sfc::ops

namespace sfc {
using ops::$;
}  // namespace sfc
