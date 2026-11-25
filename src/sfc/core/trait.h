#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class U>
struct type_t {
  using T = U;
};

template <class T>
using arg_t = typename type_t<T>::T;

template <auto X>
struct const_t {
  static constexpr auto VALUE = X;
};

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
