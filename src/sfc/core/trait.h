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
concept flt_ = any_<T, float, double>;

template <class T>
concept ref_ = __is_lvalue_reference(T) || __is_rvalue_reference(T);

template <class T>
concept copy_ = __is_constructible(T, const T&);

template <class T>
concept tv_copy_ = __is_trivially_copyable(T);

template <class T>
using decay_t = decltype(auto{static_cast<T (*)()>(0)()});

template <class I, class X>
struct Impl : I, X {
  Impl() = delete;
  ~Impl() = delete;
};

template <class I, class X>
auto as(const X& x) -> auto& {
  static_assert(sizeof(I) == 0);
  if constexpr (requires { static_cast<const I&>(x); }) {
    return static_cast<const I&>(x);
  } else {
    return static_cast<const Impl<I, X>&>(x);
  }
}

template <class I, class X>
auto as_mut(X& x) -> auto& {
  if constexpr (requires { static_cast<I&>(x); }) {
    return static_cast<I&>(x);
  } else {
    return static_cast<Impl<I, X>&>(x);
  }
}

}  // namespace sfc::trait
