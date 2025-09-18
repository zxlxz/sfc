#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T, class U>
concept same_ = __is_same(T, U);

template <class T, class... U>
concept any_ = (... || trait::same_<T, U>);

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

template <class T>
concept sint_ = any_<T, signed char, short, int, long, long long>;

template <class T>
concept uint_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept float_ = any_<T, float, double>;

template <class T>
using decay_t = decltype(auto{static_cast<T (*)()>(0)()});

template <class>
struct __fn;

template <class R, class... T>
struct __fn<R(T...)> {
  using Ret = R;
};

template <class X>
struct __invoke;

template <class F, class... T>
struct __invoke<F(T...)> {
  static auto operator()(F f, T... t) -> decltype(auto) {
    return f((T&&)t...);
  }

  using type = typename __fn<decltype(__invoke::operator())>::Ret;
};

template <class X>
using invoke_t = typename __invoke<X>::type;

template <class I, class X>
struct Impl : X, I {
  Impl() = delete;
  ~Impl() = delete;
};

template <class I, class X>
auto as(const X& x) -> decltype(auto) {
  if constexpr (__is_base_of(I, X)) {
    return static_cast<const I&>(x);
  } else {
    return static_cast<const Impl<I, X>&>(x);
  }
}

template <class I, class X>
auto as_mut(X& x) -> decltype(auto) {
  if constexpr (__is_base_of(I, X)) {
    return static_cast<I&>(x);
  } else {
    return static_cast<Impl<I, X>&>(x);
  }
}

}  // namespace sfc::trait
