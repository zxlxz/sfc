#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <auto X>
struct ConstVal {
  static constexpr auto VALUE = X;
};

#if defined(__clang__) || defined(__GNUC__)
template <class T, class U>
concept same_ = __is_same(T, U);
#else
template <class T, class U>
struct _IsSame : ConstVal<false> {};

template <class T>
struct _IsSame<T, T> : ConstVal<true> {};

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
struct idxs_t {
  template <class T>
  static auto collect(auto&& v) -> T {
    return T{v[I]...};
  }
};

#ifdef __clang__
template <u32 I, class... T>
using element_t = T...[I];
#else
template <u32 I, class... T>
struct _Element;

template <class T, class... U>
struct _Element<0, T, U...> {
  using Type = T;
};

template <u32 I, class T, class... U>
struct _Element<I, T, U...> : _Element<I - 1, U...> {};

template <u32 I, class... T>
using element_t = typename _Element<I, T...>::Type;
#endif

#ifdef __GNUC__
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
