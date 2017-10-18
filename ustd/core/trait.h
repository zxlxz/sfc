#pragma once

#include <ustd/core/type.h>

namespace ustd
{

#pragma region cond
template<bool X, class T, class F>  struct _cond;
template<bool X, class T, class F>  using  $cond = typename _cond<X, T, F>::type;

template<class T, class F>  struct _cond<true , T, F> { using type = T; };
template<class T, class F>  struct _cond<false, T, F> { using type = F; };
#pragma endregion

#pragma region when
template<bool X, class T = void> struct _when;
template<        class T       > struct _when<true, T> { using U = T; };
template<bool X, class T = void> using  $when = typename _when<X, T>::U;
#pragma endregion

#pragma region is
template<class T, class F>  struct _is      : $bool<__is_base_of(T, F)> {};
template<class T         >  struct _is<T, T>: $bool<true> {};

template<class T, class F> constexpr     bool  $is      = _is<T, F>::value;
template<class T, class F, class U=void> using $when_is = $when< _is<T, F>::value, U>;
template<class T, class F, class U=void> using $when_not= $when<!_is<T, F>::value, U>;

#pragma region trait
struct $val;        // triat: type is val
struct $ref;        // trait: type is ref
struct $ptr;        // trait: type is ptr
struct $const;      // trait: type is const
struct $mut;        // trait: type is mutable

template<typename T> struct _is<$val, T >: $bool<true > {};
template<typename T> struct _is<$val, T&>: $bool<false> {};

template<typename T> struct _is<$ref, T >: $bool<false> {};
template<typename T> struct _is<$ref, T&>: $bool<true > {};

template<typename T> struct _is<$ptr, T >: $bool<false> {};
template<typename T> struct _is<$ptr, T*>: $bool<true > {};

template<typename T> struct _is<$const, T      >: $bool<false> {};
template<typename T> struct _is<$const, T const>: $bool<true > {};

template<typename T> struct _is<$mut, T       >: $bool<true > {};
template<typename T> struct _is<$mut, T const >: $bool<false> {};

struct $enum{};     // trait: enum   type
struct $union{};    // trait: union  type
struct $struct{};   // trait: struct type
struct $class{};    // trait: class  type
struct $empty{};    // trait: empty  type
struct $value{};    // trait: value  type

template<typename T> struct _is<$enum,  T>: $bool<__is_enum (T) > {};
template<typename T> struct _is<$union, T>: $bool<__is_union(T) > {};
template<typename T> struct _is<$empty, T>: $bool<__is_empty(T) > {};
template<typename T> struct _is<$value, T>: $bool<__has_trivial_copy(T) > {};
template<typename T> struct _is<$struct,T>: $bool<__is_class(T) &&  _is<$value, T>::value > {};
template<typename T> struct _is<$class, T>: $bool<__is_class(T) && !_is<$value, T>::value > {};
#pragma endregion

#pragma endregion

#pragma region is
template<class T, class ...F> struct           _as      : $bool<__is_constructible(T, F...)> {};
template<class T, class ...F> constexpr  bool  $as      = _as<T, F...>::value;
template<class T, class F, class U=void> using $when_as = $when<_as<T, F>::value, U>;
#pragma endregion

#pragma region num-trait
template<typename T, typename U> struct _is;

struct $sint{};     // trait: type is signed integer
struct $uint{};     // trait: type is unsigned integer
struct $int{};      // trait: type is integer
struct $float{};    // trait: type is floating point
struct $number{};   // trait: type is number(integer or float) 
template<class T> struct _is<$sint  , T>: $bool<_is<i8,    T>::value || _is<i16,   T>::value  || _is<i32, T>::value || _is<i64, T>::value> {};
template<class T> struct _is<$uint  , T>: $bool<_is<u8,    T>::value || _is<u16,   T>::value  || _is<u32, T>::value || _is<u64, T>::value> {};
template<class T> struct _is<$float , T>: $bool<_is<f32,   T>::value || _is<f64,   T>::value> {};
template<class T> struct _is<$int   , T>: $bool<_is<$sint, T>::value || _is<$uint, T>::value> {};
template<class T> struct _is<$number, T>: $bool<_is<$int,  T>::value || _is<$float,T>::value> {};
#pragma endregion

#pragma region trait
template<typename T, bool X, typename ...S> struct _match {};

template<typename T, typename U, typename ...S>
struct _match<T, true, U, S...> { using type = U; };

template<typename T, typename _, typename U, typename ...S>
struct _match<T, false, _, U, S...>: _match<T, _is<U, T>::value, U, S...>  {
};

template<class T, typename U, typename ...S>
using $trait = typename _match<T, _is<U, T>::value, U, S...>::type;

#pragma endregion

}
