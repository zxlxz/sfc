#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T, class U>
concept same_ = __is_same(T, U);

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
#if __has_builtin(__is_trivially_destructible)
concept tv_dtor_ = __is_trivially_destructible(T);
#else
concept tv_dtor_ = __has_trivial_destructor(T);
#endif

template <class T>
concept polymorphic_ = __is_polymorphic(T);

template <auto... I>
struct idxs_t {
  template <class T>
  static auto collect(auto&& v) -> T {
    return T{v[I]...};
  }
};

#if defined(__GNUC__) && !defined(__clang__)
template <auto N>
using idxs_seq_t = idxs_t<__integer_pack(N)...>;
#else
template <class, auto... I>
struct _IntSeq {
  using Type = idxs_t<I...>;
};

template <auto N>
using idxs_seq_t = typename __make_integer_seq<_IntSeq, decltype(N), N>::Type;
#endif

}  // namespace sfc::trait
