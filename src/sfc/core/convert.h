#pragma once

#include "sfc/core/trait.h"

namespace sfc::convert {

template <class T, class F>
struct Ctor {
  F _func;

  auto operator()() -> T {
    return _func();
  }
};

template <class T, class... U>
auto construct(U&&... args) {
  auto f = [&]() -> T { return T{static_cast<U&&>(args)...}; };
  return Ctor<T, decltype(f)>{._func = f};
}

}  // namespace sfc::convert
