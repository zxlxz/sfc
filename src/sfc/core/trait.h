#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <auto X>
struct Const {
  static constexpr auto VALUE = X;
};

#if defined(__clang__) || defined(__GNUC__)
template <class T, class U>
concept same_ = __is_same(T, U);
#else
template <class T, class U>
struct _IsSame : Const<false> {};

template <class T>
struct _IsSame<T, T> : const<true> {};

template <class T, class U>
concept same_ = _IsSame<T, U>::VALUE;
#endif

template <class T, class... U>
concept any_ = (... || same_<T, U>);

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

template <auto... I>
struct idxs_t {};

#if __has_builtin(__make_integer_seq)
template <class, auto... I>
struct _int_seq_helper {
  using Type = idxs_t<I...>;
};

template <auto N>
using idxs_seq_t = typename __make_integer_seq<_int_seq_helper, decltype(N), N>::Type;
#else
template <auto N>
using idxs_seq_t = idxs_t<__integer_pack(N)...>;
#endif

}  // namespace sfc::trait
