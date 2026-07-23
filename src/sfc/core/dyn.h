#pragma once
#include "sfc/core/mod.h"

namespace sfc::dyn {

template<class Self>
auto cast(auto& impl) -> Self& {
  return *ptr::cast<Self>(&impl);
}

template <auto f>
struct Fn {
  template <class Self, class R, class... T>
  using fun_t = R (*)(Self&, T...);

  template <class Self, class R, class... T>
  operator fun_t<Self, R, T...>() const {
    auto conv = []<class X>(R (X::*)(T...)) {
      return [](Self& self, T... t) -> R { return ((X&)self.*f)((T&&)t...); };
    };
    return conv(f);
  }
};

}  // namespace sfc::dyn
