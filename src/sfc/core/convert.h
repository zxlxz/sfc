#pragma once

#include "sfc/core/trait.h"

namespace sfc::convert {

template <class T, class F>
class Into {
  F _func;

 public:
  Into(auto& func) noexcept : _func{func} {}
  Into(const Into&) = delete;

 public:
  auto operator()() && -> T {
    return _func();
  }
};

template <class T, class... U>
auto into(U&&... args) {
  auto f = [&args...]() -> T { return T{static_cast<U&&>(args)...}; };
  return Into<T, decltype(f)>{f};
}

}  // namespace sfc::convert
