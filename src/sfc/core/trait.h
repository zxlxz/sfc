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
concept sint_ = any_<T, char, signed char, short, int, long, long long>;

template <class T>
concept uint_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept flt_ = any_<T, float, double, long double>;

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

template <auto... I>
struct idxs_t {};

template <class, auto... I>
struct _int_seq_helper {
  using Type = idxs_t<I...>;
};

template <auto N>
using idxs_seq_t = typename __make_integer_seq<_int_seq_helper, decltype(N), N>::Type;

template <class I, class X>
auto as(const X& x) -> auto& {
  struct Impl : I, X {};
  if constexpr (requires { static_cast<const I&>(x); }) {
    return static_cast<const I&>(x);
  } else {
    return static_cast<const Impl&>(x);
  }
}

template <class I, class X>
auto as_mut(X& x) -> auto& {
  struct Impl : I, X {};
  if constexpr (requires { static_cast<I&>(x); }) {
    return static_cast<I&>(x);
  } else {
    return static_cast<Impl&>(x);
  }
}

}  // namespace sfc::trait
