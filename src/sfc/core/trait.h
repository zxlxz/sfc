#pragma once

#include "mod.h"

namespace sfc::trait {

struct Any {};

template <class T, class U>
concept Same = __is_same(T, U);

template <class T, class U>
concept NotSame = !__is_same(T, U);

template <class T>
concept Enum = __is_enum(T);

template <class T>
concept Class = __is_class(T);

template <class T>
concept Copy = __is_trivially_copyable(T);

template <class T>
concept UInt = __is_same(T, unsigned char)      //
               || __is_same(T, unsigned short)  //
               || __is_same(T, unsigned int)    //
               || __is_same(T, unsigned long)   //
               || __is_same(T, unsigned long long);

template <class T>
concept SInt = __is_same(T, signed char)      //
               || __is_same(T, signed short)  //
               || __is_same(T, signed int)    //
               || __is_same(T, signed long)   //
               || __is_same(T, signed long long);

template <class T>
concept Int = UInt<T> || SInt<T>;

template <class T>
concept Float = __is_same(T, float)      //
                || __is_same(T, double)  //
                || __is_same(T, long double);

template <class T>
concept Reference = __is_same(T, T&);

template <template <class> class X, class T>
auto as(const T& x) -> const X<T>& {
  return reinterpret_cast<const X<T>&>(x);
}

template <template <class> class X, class T>
auto as_mut(T& x) -> X<T>& {
  return reinterpret_cast<X<T>&>(x);
}

}  // namespace sfc::trait
