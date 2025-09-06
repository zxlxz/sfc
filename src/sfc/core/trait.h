#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T, class U>
concept is_ = __is_same(T, U);

template <class T, class... U>
concept any_ = (... || trait::is_<T, U>);

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

template <class T>
concept float_ = any_<T, float, double, long double>;

template <class T>
concept signed_ = any_<T, signed char, short, int, long, long long>;

template <class T>
concept unsigned_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = signed_<T> || unsigned_<T>;

}  // namespace sfc::trait
