#pragma once

#include "mod.h"

namespace sfc::trait {

template <class T>
concept isEnum = __is_enum(T);

template <class T>
concept isUnion = __is_union(T);

template <class T>
concept isClass = __is_class(T);

template <class T>
concept isRef = __is_same(T, T&);

template <class T>
concept isConst = __is_same(T, const T);

template <class T>
concept isUInt =
    __is_same(T, unsigned char) || __is_same(T, unsigned short) || __is_same(T, unsigned int) ||
    __is_same(T, unsigned long) || __is_same(T, unsigned long long);

template <class T>
concept isSInt = __is_same(T, signed char) || __is_same(T, short) || __is_same(T, int) ||
                 __is_same(T, long) || __is_same(T, long long);

template <class T>
concept isInt = isUInt<T> || isSInt<T>;

template <class T>
concept isFlt = __is_same(T, float) || __is_same(T, double);

template <class T>
concept isNum = isInt<T> || isFlt<T>;

template <template <class> class X, class T>
auto as(const T& x) -> const X<T>& {
  return reinterpret_cast<const X<T>&>(x);
}

template <template <class> class X, class T>
auto as_mut(T& x) -> X<T>& {
  return reinterpret_cast<X<T>&>(x);
}

}  // namespace sfc::trait
