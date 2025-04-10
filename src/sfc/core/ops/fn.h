#pragma once

#include "sfc/core/trait.h"

namespace sfc::ops {

template <class>
struct IFn;

template <class R, class... T>
struct IFn<R(T...)> {
  struct Meta {
    void (*_drop)(void*) = nullptr;
    R (*_call)(void*, T&&...) = nullptr;
  };

  template <class X>
  static inline const Meta META = {
      ._drop = [](void* p) { return (*static_cast<X*>(p)).~X(); },
      ._call = [](void* p, T&&... args) { return (*(X*)(p))((T&&)(args)...); },
  };

  void*       _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  IFn() = default;

  template <class X>
  explicit IFn(X& x) noexcept : _self{&x}, _meta{&META<X>} {}

  auto operator()(T&&... args) -> R {
    return (_meta->_call)(_self, static_cast<T&&>(args)...);
  }
};

template <class I, class X, class R, class... T>
auto dyn_fn(R (X::*f)(T...)) {
  return reinterpret_cast<R (I::*)(T...)>(f);
};

template <class I, class X, class R, class... T>
auto dyn_fn(R (X::*f)(T...) const) {
  return reinterpret_cast<R (I::*)(T...) const>(f);
};

template <class I, class M, class X, class R, class... T, class... F>
auto dyn_meta(R (X::*f0)(T...), F... fs) -> M {
  auto drop = [](void* p) { return static_cast<X*>(p)->~X(); };
  return {drop, dyn_fn<I>(f0), dyn_fn<I>(fs)...};
}

}  // namespace sfc::ops

namespace sfc {
using ops::IFn;
}  // namespace sfc
