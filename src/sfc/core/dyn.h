#pragma once
#include "sfc/core/mod.h"

namespace sfc::dyn {

template <class Self>
auto cast(auto& impl) -> Self& {
  return (Self&)(impl);
}

template <auto f>
auto fn(auto& x) {
  auto conv = []<class Self, class X, class R, class... T>(Self&, R (X::*)(T...)) {
    using F = R (*)(Self&, T...);
    return F{[](Self& self, T... t) -> R { return ((X&)self.*f)((T&&)t...); }};
  };
  return conv(x, f);
}

}  // namespace sfc::dyn
