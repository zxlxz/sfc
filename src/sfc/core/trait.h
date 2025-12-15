#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T, class U>
concept same_ = __is_same(T, U);

template <class T, class... U>
concept any_ = (... || __is_same(T, U));

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

template <class T>
concept int_ = __is_integral(T);

template <class T>
concept flt_ = __is_floating_point(T);

template <class T>
concept sint_ = __is_integral(T) && __is_signed(T);

template <class T>
concept uint_ = __is_integral(T) && __is_unsigned(T);

template <class T>
concept ptr_ = __is_pointer(T);

template <class T>
concept ref_ = __is_lvalue_reference(T) || __is_rvalue_reference(T);

template <class T>
concept copy_ = __is_constructible(T, const T&);

template <class T>
concept tv_copy_ = __is_trivially_copyable(T);

template <class T>
concept tv_dtor_ = __is_trivially_destructible(T);

template <class T>
concept polymorphic_ = __is_polymorphic(T);

template <class F, class T>
concept AsRef = requires(const F& from) { T{from}; };

template <class I, class X>
struct Impl : I, X {
  static_assert(__is_empty(I), "Trait interface must be empty");
};

template <class I, class X>
auto as(const X& x) -> auto& {
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
