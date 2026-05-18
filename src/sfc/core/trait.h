#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T>
struct type_t {};

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
concept sint_ = same_<T, signed char> || same_<T, short> || same_<T, int> || same_<T, long> ||
                same_<T, long long>;

template <class T>
concept uint_ = same_<T, unsigned char> || same_<T, unsigned short> || same_<T, unsigned int> ||
                same_<T, unsigned long> || same_<T, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept float_ = same_<T, float> || same_<T, double>;

}  // namespace sfc::trait

namespace sfc {
using trait::same_;
using trait::enum_;
using trait::class_;

using trait::int_;
using trait::sint_;
using trait::uint_;
using trait::float_;
}  // namespace sfc
