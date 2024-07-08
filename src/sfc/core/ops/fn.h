#pragma once

#include "sfc/core/mod.h"

namespace sfc::ops {

struct Any {
  template <class X, class R, class... T>
  static auto cast(R (X::*f)(T...)) -> R (Any::*)(T...) {
    return reinterpret_cast<R (Any::*)(T...)>(f);
  }

  template <class X, class R, class... T>
  static auto cast(R (X::*f)(T...) const) -> R (Any::*)(T...) {
    return reinterpret_cast<R (Any::*)(T...)>(f);
  }
};

template <class F>
struct Fn;

template <class R, class... T>
struct Fn<R(T...)> {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    R (Any::*_call)(T...);

    template <class X>
    static auto of(const X&) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._call = Any::cast<X, R, T...>(&X::operator()),
      };
      return res;
    }
  };

  Any* _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  Fn() noexcept = default;

  template <class X>
  explicit Fn(X& x) : _self{reinterpret_cast<Any*>(&x)}, _meta{&Meta::of(x)} {}
};

}  // namespace sfc::ops

namespace sfc {
using ops::Any;
using ops::Fn;
}  // namespace sfc
