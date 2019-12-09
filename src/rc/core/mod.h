#pragma once

#ifdef _MSC_VER
#include <corecrt.h>
#include <string.h>
#pragma warning(disable : 4201)
#pragma warning(disable : 4623 4626 5027)
#pragma warning(disable : 4820)

#ifndef __clang__
#define __builtin_memcpy  ::memcpy
#define __builtin_memmove ::memmove
#define __builtin_memset  ::memset
#endif

#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmicrosoft-template"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wgcc-compat"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wundefined-func-template"
#pragma clang diagnostic ignored "-Wc99-extensions"
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wc++98-c++11-c++14-compat"
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

#ifdef __INTELLISENSE__
#define __attribute__(...)
#endif

#ifndef pub
#define pub
#endif

namespace rc {

using i8 = char;
using u8 = unsigned char;

using i16 = short;
using u16 = unsigned short;

using i32 = int;
using u32 = unsigned int;

using i64 = decltype(0x1FFFFFFFFFFFFFFF);
using u64 = decltype(0xFFFFFFFFFFFFFFFF);

using usize = decltype(sizeof(0));
using isize = decltype(static_cast<u8*>(nullptr) - static_cast<u8*>(nullptr));

using f32 = float;
using f64 = double;

using cstr_t = const char*;

template <class T, class... U>
using type_t = T;

template <class... U>
using void_t = void;

template <class... U>
using bool_t = bool;

struct unit {};

struct f16 {
  u16 _val;
};

namespace option {
template <class T>
struct Option;
}

namespace slice {
template <class T>
struct Slice;
}

namespace str {
struct Str;
}
#pragma region macros

template <class... T>
inline auto panic(const T&... args) -> void;

template <class... Args>
inline auto assert(bool x, const Args&... args) -> void;

template <class A, class B>
inline auto assert_eq(const A& a, const B& b) -> void;

template <class A, class B>
inline auto assert_ne(const A& a, const B& b) -> void;
#pragma endregion

#pragma region tuple
template <class...>
struct Tuple;

template <>
struct Tuple<> {};

template <class T0>
struct Tuple<T0> {
  T0 _0;
};

template <class T0, class T1>
struct Tuple<T0, T1> {
  T0 _0;
  T1 _1;
};

template <class T0, class T1, class T2>
struct Tuple<T0, T1, T2> {
  T0 _0;
  T1 _1;
  T2 _2;
};

template <class T0, class T1, class T2, class T3>
struct Tuple<T0, T1, T2, T3> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
};

template <class T0, class T1, class T2, class T3, class T4>
struct Tuple<T0, T1, T2, T3, T4> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
};

template <class T0, class T1, class T2, class T3, class T4, class T5>
struct Tuple<T0, T1, T2, T3, T4, T5> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
  T5 _5;
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
struct Tuple<T0, T1, T2, T3, T4, T5, T6> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
  T5 _5;
  T6 _6;
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
  T5 _5;
  T6 _6;
  T7 _7;
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
  T5 _5;
  T6 _6;
  T7 _7;
  T8 _8;
};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8,
          class T9>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> {
  T0 _0;
  T1 _1;
  T2 _2;
  T3 _3;
  T4 _4;
  T5 _5;
  T6 _6;
  T7 _7;
  T8 _8;
  T9 _9;
};

template <class... T>
Tuple(const T&...)->Tuple<T...>;

#pragma endregion

#pragma region const
template <auto... I>
struct Const {};

template <auto I>
struct Const<I> {
  constexpr static auto VALUE = I;
};

template <class T, T... I>
struct _ConstSeq {
  using Type = Const<I...>;
};

template <auto N>
using const_seq_t = typename __make_integer_seq<_ConstSeq, decltype(N), N>::Type;

template <class I, usize N, bool... X>
struct _ConstFind;

template <usize... I, usize N>
struct _ConstFind<Const<I...>, N> {
  using Type = Const<I...>;
};

template <usize... I, usize N, bool... X>
struct _ConstFind<Const<I...>, N, true, X...> : _ConstFind<Const<I..., N>, N + 1, X...> {};

template <usize... I, usize N, bool... X>
struct _ConstFind<Const<I...>, N, false, X...> : _ConstFind<Const<I...>, N + 1, X...> {};

template <bool... X>
using const_find_t = typename _ConstFind<Const<>, 0, X...>::Type;

#pragma endregion

#pragma region cast

template <class T>
inline auto move(T& ref) noexcept -> T&& {
  return static_cast<T&&>(ref);
}

template <class T>
inline auto declptr() -> T* {
  return static_cast<T*>(nullptr);
}

template <class T>
inline auto declval() -> T {
  return static_cast<T&&>(*static_cast<T*>(nullptr));
}
#pragma endregion

#pragma region where
template <bool X, class T = void>
struct _Where;

template <class T>
struct _Where<true, T> {
  using type = T;
};

template <bool X, class T = void>
using where_t = typename _Where<true, T>::type;

#pragma endregion

#pragma region trait

template <class T, class U>
struct _IsSame : Const<false> {};

template <class T>
struct _IsSame<T, T> : Const<true> {};

template <class T>
struct _IsRef : Const<false> {};

template <class T>
struct _IsRef<T&> : Const<true> {};

template <class T>
struct _IsRef<T&&> : Const<true> {};

template <class T>
struct _IsConst : Const<false> {};

template <class T>
struct _IsConst<const T> : Const<true> {};

template <class T, class U>
constexpr bool is_same() {
#ifndef __clang__
  return _IsSame<T, U>::VALUE;
#else
  return __is_same(T, U);
#endif
}

template <class T>
constexpr bool is_ref() {
  return _IsRef<T>::VALUE;
}

template <class T>
constexpr bool is_const() {
  return _IsConst<T>::VALUE;
}

template <class T>
constexpr bool is_signed() {
  return rc::is_same<T, char>() || rc::is_same<T, short>() || rc::is_same<T, int>() ||
         rc::is_same<T, long>() || rc::is_same<T, long long>();
}

template <class T>
constexpr bool is_unsigned() {
  return rc::is_same<T, u8>() || rc::is_same<T, unsigned char>() ||
         rc::is_same<T, unsigned short>() || rc::is_same<T, unsigned int>() ||
         rc::is_same<T, unsigned long>() || rc::is_same<T, unsigned long long>();
}

template <class T>
constexpr bool is_floating_point() {
  return rc::is_same<T, f32>() || rc::is_same<T, f64>();
}

template <class T>
constexpr bool is_integeral() {
  return rc::is_signed<T>() || rc::is_unsigned<T>();
}

template <class T>
constexpr bool is_num() {
  return rc::is_integeral<T>() || rc::is_floating_point<T>();
}

template<class T>
constexpr bool is_char() {
  return rc::is_same<T, char>() || rc::is_same<T, char8_t>() ||
         rc::is_same<T, char16_t>() || rc::is_same<T, char32_t>();
}
#pragma endregion

#pragma region choose

template <usize I, class... T>
struct _Select;

template <class T, class... U>
struct _Select<0, T, U...> {
  using Type = T;
};

template <usize I, class T, class... U>
struct _Select<I, T, U...> : _Select<I - 1, U...> {};

template <auto I, class... T>
using select_t = typename _Select<I, T...>::Type;

template <bool X, class T0, class T1>
using choose_t = typename _Select<(X ? 0 : 1), T0, T1>::Type;

#pragma endregion

#pragma region types

template <class T>
struct _RemoveConst {
  using Type = T;
};

template <class T>
struct _RemoveConst<const T> {
  using Type = T;
};

template <class T>
struct _RemoveConst<const T&> {
  using Type = T&;
};

template <class T>
struct _RemoveRef {
  using Type = T;
};

template <class T>
struct _RemoveRef<T&> {
  using Type = T;
};

template <class T>
struct _RemoveRef<T&&> {
  using Type = T;
};

template <class T>
using remove_const_t = typename rc::_RemoveConst<T>::Type;

template <class T>
using remove_ref_t = typename rc::_RemoveRef<T>::Type;
#pragma endregion

}  // namespace rc
