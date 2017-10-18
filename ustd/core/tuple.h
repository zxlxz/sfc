#pragma once

#include <ustd/core/type.h>
#include <ustd/core/slice.h>
#include <ustd/core/str.h>
#include <ustd/core/macro.h>

namespace ustd
{

template<class ...T> struct Tuple;

#define ustd_tuple_decl(n, ...) class T##n,
#define ustd_tuple_type(n, ...) T##n,
#define ustd_tuple_body(n, ...) T##n $##n;                                  \
    fn operator[]($usize<n>)       noexcept -> T##n&       { return $##n; } \
    fn operator[]($usize<n>) const noexcept -> const T##n& { return $##n; }

template<USTD_LOOP_0 (ustd_tuple_decl) class T0 > struct Tuple<USTD_LOOP_0 (ustd_tuple_type) T0 > { USTD_LOOP_1 (ustd_tuple_body) };
template<USTD_LOOP_1 (ustd_tuple_decl) class T1 > struct Tuple<USTD_LOOP_1 (ustd_tuple_type) T1 > { USTD_LOOP_2 (ustd_tuple_body) };
template<USTD_LOOP_2 (ustd_tuple_decl) class T2 > struct Tuple<USTD_LOOP_2 (ustd_tuple_type) T2 > { USTD_LOOP_3 (ustd_tuple_body) };
template<USTD_LOOP_3 (ustd_tuple_decl) class T3 > struct Tuple<USTD_LOOP_3 (ustd_tuple_type) T3 > { USTD_LOOP_4 (ustd_tuple_body) };
template<USTD_LOOP_4 (ustd_tuple_decl) class T4 > struct Tuple<USTD_LOOP_4 (ustd_tuple_type) T4 > { USTD_LOOP_5 (ustd_tuple_body) };
template<USTD_LOOP_5 (ustd_tuple_decl) class T5 > struct Tuple<USTD_LOOP_5 (ustd_tuple_type) T5 > { USTD_LOOP_6 (ustd_tuple_body) };
template<USTD_LOOP_6 (ustd_tuple_decl) class T6 > struct Tuple<USTD_LOOP_6 (ustd_tuple_type) T6 > { USTD_LOOP_7 (ustd_tuple_body) };
template<USTD_LOOP_7 (ustd_tuple_decl) class T7 > struct Tuple<USTD_LOOP_7 (ustd_tuple_type) T7 > { USTD_LOOP_8 (ustd_tuple_body) };
template<USTD_LOOP_8 (ustd_tuple_decl) class T8 > struct Tuple<USTD_LOOP_8 (ustd_tuple_type) T8 > { USTD_LOOP_9 (ustd_tuple_body) };
template<USTD_LOOP_9 (ustd_tuple_decl) class T9 > struct Tuple<USTD_LOOP_9 (ustd_tuple_type) T9 > { USTD_LOOP_10(ustd_tuple_body) };
template<USTD_LOOP_10(ustd_tuple_decl) class T10> struct Tuple<USTD_LOOP_10(ustd_tuple_type) T10> { USTD_LOOP_11(ustd_tuple_body) };
template<USTD_LOOP_11(ustd_tuple_decl) class T11> struct Tuple<USTD_LOOP_11(ustd_tuple_type) T11> { USTD_LOOP_12(ustd_tuple_body) };
template<USTD_LOOP_12(ustd_tuple_decl) class T12> struct Tuple<USTD_LOOP_12(ustd_tuple_type) T12> { USTD_LOOP_13(ustd_tuple_body) };
template<USTD_LOOP_13(ustd_tuple_decl) class T13> struct Tuple<USTD_LOOP_13(ustd_tuple_type) T13> { USTD_LOOP_14(ustd_tuple_body) };
template<USTD_LOOP_14(ustd_tuple_decl) class T14> struct Tuple<USTD_LOOP_14(ustd_tuple_type) T14> { USTD_LOOP_15(ustd_tuple_body) };
template<USTD_LOOP_15(ustd_tuple_decl) class T15> struct Tuple<USTD_LOOP_15(ustd_tuple_type) T15> { USTD_LOOP_16(ustd_tuple_body) };
template<USTD_LOOP_16(ustd_tuple_decl) class T16> struct Tuple<USTD_LOOP_16(ustd_tuple_type) T16> { USTD_LOOP_17(ustd_tuple_body) };
template<USTD_LOOP_17(ustd_tuple_decl) class T17> struct Tuple<USTD_LOOP_17(ustd_tuple_type) T17> { USTD_LOOP_18(ustd_tuple_body) };
template<USTD_LOOP_18(ustd_tuple_decl) class T18> struct Tuple<USTD_LOOP_18(ustd_tuple_type) T18> { USTD_LOOP_19(ustd_tuple_body) };
template<USTD_LOOP_19(ustd_tuple_decl) class T19> struct Tuple<USTD_LOOP_19(ustd_tuple_type) T19> { USTD_LOOP_20(ustd_tuple_body) };

#undef ustd_tuple_decl
#undef ustd_tuple_type
#undef ustd_tuple_body

template<class T>           struct _tuple_element           { using type = T;           };
template<class T, usize N>  struct _tuple_element<T(&)[N]>  { using type = Slice<T, 0>; };

template<class ...T>
fn tuple(T&& ...args) -> Tuple<typename _tuple_element<T>::type ...> {
    return { typename _tuple_element<T>::type(args)... };
}

}
