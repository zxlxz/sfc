#pragma once

#include <nms/config.h>

namespace nms
{

#pragma region builtin types
using   i8          = int8_t;       using   u8      = uint8_t;
using   i16         = int16_t;      using   u16     = uint16_t;
using   i32         = int32_t;      using   u32     = uint32_t;
using   i64         = int64_t;      using   u64     = uint64_t;

using   byte        = int8_t;
using   ubyte       = uint8_t;
using   f32         = float;
using   f64         = double;

using   uchar       = unsigned char;
using   ushort      = unsigned short;
using   uint        = unsigned int;
using   ulong       = unsigned long;
using   llong       = long long;
using   ullong      = unsigned long long;
using   nullptr_t   = decltype(nullptr);

#pragma endregion

#pragma region predef types
template<class T, u32 N = 0>
struct View;

template<class T>
class IList;

using StrView   = View<const char>;
using IString   = IList<char>;
#pragma endregion

#pragma region type modifiers
template<class T> struct _Tconst            { using U = const T; };
template<class T> struct _Tconst<const T>   { using U = const T; };

template<class T> struct _Tmutable          { using U = T;       };
template<class T> struct _Tmutable<const T> { using U = T;       };

template<class T> struct _Treference        { using U = T&;      };
template<class T> struct _Treference<T&>    { using U = T&;      };

template<class T> struct _Tvalue            { using U = T;       };
template<class T> struct _Tvalue<T&>        { using U = T;       };

template<class T> using Tconst      = typename _Tconst<T>::U;
template<class T> using Tmutable    = typename _Tmutable<T>::U;
template<class T> using Treference  = typename _Treference<T>::U;
template<class T> using Tvalue      = typename _Tvalue<T>::U;

template<class T>
__forceinline T& mutable_cast(const T& v) noexcept {
    return const_cast<T&>(v);
}

template<bool X, class T, class U>
struct _Tcond;

template<class A, class B> struct _Tcond<true,  A, B> { using U = A; };
template<class A, class B> struct _Tcond<false, A, B> { using U = B; };

template<bool  X, class A, class B=void>
using Tcond = typename _Tcond<X, A, B>::U;
#pragma endregion

#pragma region type cast
template<class T> constexpr auto  declptr() { using P = Tvalue<T>*; return  P{}; }
template<class T> constexpr auto& declval() { using P = Tvalue<T>*; return *P{}; }

template<class T> constexpr T*  $declptr = nullptr;
template<class T> constexpr T&  $declval = *static_cast<T*>(nullptr);

template<class T> __forceinline constexpr T&& fwd(Tvalue<T>&  t) noexcept { return (static_cast<T&&>(t)); }
template<class T> __forceinline constexpr T&& fwd(Tvalue<T>&& t) noexcept { return (static_cast<T&&>(t)); }

#ifdef NMS_STDCXX_UTILITY
using std::move;
#else
template<class T> __forceinline constexpr T&& move(T&  val) noexcept { return static_cast<T&&>(val); }
template<class T> __forceinline constexpr T&& move(T&& val) noexcept { return static_cast<T&&>(val); }
#endif

#pragma endregion

#pragma region swap
#ifdef NMS_STDCXX_UTILITY
using std::swap;
#else
template<class T>
__forceinline void swap(T& a, T& b) noexcept {
    T c(static_cast<T&&>(a));
    a = static_cast<T&&>(b);
    b = static_cast<T&&>(c);
}

template<class T, u32 N>
__forceinline void swap(T(&a)[N], T(&b)[N]) noexcept {
    for (u32 i = 0; i < N; ++i) {
        swap(a[i], b[i]);
    }
}
#endif
#pragma endregion

#pragma region array
/* get elements count of this array */
template<class T, u32 N>
__forceinline constexpr u32 numel(const T(&/*array*/)[N]) {
    return N;
}
#pragma endregion

#pragma region math: reduce
template<class T>
constexpr auto (sum)(T t) {
    return t;
}

template<class T, class U, class ...S>
constexpr auto (sum)(T t, U u, S ...s) {
    return t + (sum)(u, s...);
}

template<class T>
constexpr auto (prod)(T t) {
    return t;
}

template<class T, class U, class ...S>
constexpr auto (prod)(T t, U u, S ...s) {
    return t * (prod)(u, s...);
}

template<class T>
constexpr auto (max)(T t) {
    return t;
}

template<class T, class U, class ...S>
constexpr auto (max)(T t, U u, S ...s) {
    return t > (max)(u, s...) ? t : (max)(u, s...);
}

template<class T>
constexpr auto (min)(T t) {
    return t;
}

template<class T, class U, class ...S>
constexpr auto (min)(T t, U u, S ...s) {
    return t < (min)(u, s...) ? t : (min)(u, s...);
}

#pragma endregion

#pragma region static init
template<class Tret, Tret(*func)(), class Tid>
struct StaticInitor
{
    StaticInitor() {
        (void)value;
    }

    static Tret value;
};

template<class Tret, Tret(*func)(), class Tid>
Tret StaticInitor<Tret, func, Tid>::value = (*func)();

template<class Tid, u32(*func)()>
u32 static_init() {
    StaticInitor<u32, func, Tid> static_initor;
    return static_initor.value;
}
#pragma endregion

}
