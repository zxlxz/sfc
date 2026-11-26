#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class U>
struct type_t {
  using T = U;
};

template <class T>
using arg_t = typename type_t<T>::T;

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

#if defined(__clang__) || defined(__GNUC__)
template <class T, class U>
concept same_ = __is_same(T, U);
#else
template <class T, class U>
concept same_ = __is_convertible_to(type_t<T>*, type_t<U>*);
#endif

template <class T>
concept array_ = __is_array(T);

template <class T, class U>
concept not_ = !same_<T, U>;

template <class T, class... U>
concept any_ = (same_<T, U> || ...);

template <class T>
concept sint_ = same_<T, signed char> || same_<T, short> || same_<T, int> || same_<T, long> || same_<T, long long>;

template <class T>
concept uint_ = same_<T, unsigned char> || same_<T, unsigned short> || same_<T, unsigned int> ||
                same_<T, unsigned long> || same_<T, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept flt_ = same_<T, float> || same_<T, double>;

template <class T>
concept num_ = int_<T> || flt_<T>;

template <class T>
concept tv_copy_ = __is_trivially_copyable(T);

#ifdef __clang__
template <class T>
concept tv_drop_ = __is_trivially_destructible(T);
#else
template <class T>
concept tv_drop_ = __has_trivial_destructor(T);
#endif

template <auto X>
struct const_t {
  static constexpr auto VALUE = X;
};

template <u32 I>
struct const_t<I> {
  static constexpr auto VALUE = I;
};

template <u32 I>
struct index_t {
  static constexpr auto VALUE = I;

  static auto operator[](auto&& obj) -> auto& {
    static_assert(I < 16, "field index out of range");
    if constexpr (I == 0) return obj._0;
    if constexpr (I == 1) return obj._1;
    if constexpr (I == 2) return obj._2;
    if constexpr (I == 3) return obj._3;
    if constexpr (I == 4) return obj._4;
    if constexpr (I == 5) return obj._5;
    if constexpr (I == 6) return obj._6;
    if constexpr (I == 7) return obj._7;
    if constexpr (I == 8) return obj._8;
    if constexpr (I == 9) return obj._9;
    if constexpr (I == 10) return obj._10;
    if constexpr (I == 11) return obj._11;
    if constexpr (I == 12) return obj._12;
    if constexpr (I == 13) return obj._13;
    if constexpr (I == 14) return obj._14;
    if constexpr (I == 15) return obj._15;
  }
};

template <u32 BEGIN, u32 END>
void for_each(auto&& f) {
  if constexpr (BEGIN < END) {
    f(index_t<BEGIN>{});
    for_each<BEGIN + 1, END>(f);
  }
}

}  // namespace sfc::trait

namespace sfc {
using trait::type_t;
using trait::arg_t;
using trait::const_t;

using trait::same_;
using trait::enum_;
using trait::class_;

using trait::tv_copy_;
using trait::tv_drop_;

using trait::int_;
using trait::sint_;
using trait::uint_;
using trait::flt_;
}  // namespace sfc
