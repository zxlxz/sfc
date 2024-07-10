#pragma once

#include "sfc/core/trait.h"

namespace sfc::ops {

template <class X, class R, class... T>
static auto fn(R (X::*f)(T...)) -> R (trait::Any::*)(T...) {
  return reinterpret_cast<R (trait::Any::*)(T...)>(f);
}

template <class X, class R, class... T>
static auto fn(R (X::*f)(T...) const) -> R (trait::Any::*)(T...) {
  return reinterpret_cast<R (trait::Any::*)(T...)>(f);
}

template <class F>
struct Fn;

template <class R, class... T>
struct Fn<R(T...)> {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    R (trait::Any::*_call)(T...);

    template <class X>
    static auto of(const X*) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._call = ops::fn<X, R, T...>(&X::operator()),
      };
      return res;
    }
  };

  void* _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  Fn() noexcept = default;

  Fn(auto* x) : _self{x}, _meta{&Meta::of(x)} {}
};

}  // namespace sfc::ops

namespace sfc {
using ops::Fn;
}  // namespace sfc
