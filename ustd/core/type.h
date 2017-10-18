#pragma once

#include <ustd/core/_.h>

namespace ustd
{

#pragma region types: primitive
using i8  = __int8;     using u8  = unsigned __int8;
using i16 = __int16;    using u16 = unsigned __int16;
using i32 = __int32;    using u32 = unsigned __int32;
using i64 = __int64;    using u64 = unsigned __int64;

using f32 = float;
using f64 = double;

using byte      = i8;
using llong     = long long;

using ubyte     = u8;
using ushort    = unsigned short;
using uint      = unsigned int;
using ulong     = unsigned long;
using ullong    = unsigned long long;

using isize     = decltype(static_cast<char*>(nullptr) - static_cast<char*>(nullptr));
using usize     = decltype(sizeof(0));

template<typename T> constexpr fn declptr() noexcept -> T* { return  static_cast<T*>(nullptr); }
template<typename T> constexpr fn declval() noexcept -> T& { return *static_cast<T*>(nullptr); }
#pragma endregion

#pragma region types: vec
#ifdef __clang__
typedef i8  i8x2[[gnu::__vector_size__(2)]];
typedef i8  i8x4[[gnu::__vector_size__(4)]];
typedef i8  i8x8[[gnu::__vector_size__(8)]];

typedef u8  u8x2[[gnu::__vector_size__(2)]];
typedef u8  u8x4[[gnu::__vector_size__(4)]];
typedef u8  u8x8[[gnu::__vector_size__(8)]];

typedef i16 i16x2[[gnu::__vector_size__(4)]];
typedef i16 i16x4[[gnu::__vector_size__(8)]];
typedef i16 i16x8[[gnu::__vector_size__(16)]];

typedef u16 u16x2[[gnu::__vector_size__(4)]];
typedef u16 u16x4[[gnu::__vector_size__(8)]];
typedef u16 u16x8[[gnu::__vector_size__(16)]];

typedef i32 i32x2[[gnu::__vector_size__(8)]];
typedef i32 i32x4[[gnu::__vector_size__(16)]];
typedef i32 i32x8[[gnu::__vector_size__(32)]];

typedef u32 u32x2[[gnu::__vector_size__(8)]];
typedef u32 u32x4[[gnu::__vector_size__(16)]];
typedef u32 u32x8[[gnu::__vector_size__(32)]];

typedef f32 f32x2[[gnu::__vector_size__(8)]];
typedef f32 f32x4[[gnu::__vector_size__(16)]];
typedef f32 f32x8[[gnu::__vector_size__(32)]];

typedef f64 f64x2[[gnu::__vector_size__(16)]];
typedef f64 f64x4[[gnu::__vector_size__(32)]];
#endif
#pragma endregion

#pragma region $type
template<class T=void, T ...v> struct $type {};

template<usize ...v> using $usize = $type<usize, v...>;
template<isize ...v> using $isize = $type<usize, v...>;
template<bool  ...v> using $bool  = $type<bool,  v...>;
template<u32   ...v> using $u32   = $type<u32,   v...>;
template<i32   ...v> using $i32   = $type<i32,   v...>;

template<class T, T v>
struct $type<T, v>
{
    static constexpr T value = v;
};
#pragma endregion

#pragma region index_seq
template<usize N> using $index_seq  = __make_integer_seq<$type, usize, N>;
#pragma endregion

#pragma region index_if
template<usize K, class    I, bool ...V> struct _index_if_t;
template<usize K, usize ...I           > struct _index_if_t<K, $usize<I...>             > { using U = $usize<I...>; };
template<usize K, usize ...I, bool ...V> struct _index_if_t<K, $usize<I...>, true,  V...> { using U = typename _index_if_t<K + 1, $usize<I..., K>, V...>::U; };
template<usize K, usize ...I, bool ...V> struct _index_if_t<K, $usize<I...>, false, V...> { using U = typename _index_if_t<K + 1, $usize<I...   >, V...>::U; };
template<bool ...V> using $index_if  = typename _index_if_t<0, $usize<>, V...>::U;
#pragma endregion

#pragma region funcs: convert
template<typename T> struct _val_t { using U = T; };
template<typename T> struct _val_t<T&> { using U = T; };

template<typename T> struct _ref_t { using U = T&; };
template<typename T> struct _ref_t<T&> { using U = T&; };

template<typename T> struct _mut_t { using U = T; };
template<typename T> struct _mut_t<const T> { using U = T; };

template<typename T> struct _const_t { using U = const T; };
template<typename T> struct _const_t<const T> { using U = const T; };

template<typename T> using val_t    = typename _val_t<T>::U;
template<typename T> using ref_t    = typename _ref_t<T>::U;
template<typename T> using mut_t    = typename _mut_t<T>::U;
template<typename T> using const_t  = typename _const_t<T>::U;

template<typename T> __forceinline constexpr fn as_ref(T& ref) noexcept -> T& { return ref; }
template<typename T> __forceinline constexpr fn as_val(T& ref) noexcept -> T { return ref; }

template<typename T> __forceinline constexpr fn as_mut(const T& ref) noexcept -> T& { return const_cast<T&>(ref); }
template<typename T> __forceinline constexpr fn as_mut(const T* ptr) noexcept -> T* { return const_cast<T*>(ptr); }

template<typename T> __forceinline constexpr fn as_mov(T&  ref) noexcept->T&& { return static_cast<T&&>(ref); }
template<typename T> __forceinline constexpr fn as_mov(T&& ref) noexcept->T&& { return ref; }

template<typename T> __forceinline constexpr fn as_const(const T& ref) noexcept -> const T&{ return ref; }
template<typename T> __forceinline constexpr fn as_const(const T* ptr) noexcept -> const T*{ return ptr; }

template<typename T> __forceinline constexpr fn as_fwd(val_t<T>&  ref) noexcept -> T&& { return static_cast<T&&>(ref); }
template<typename T> __forceinline constexpr fn as_fwd(val_t<T>&& ref) noexcept -> T&& { return static_cast<T&&>(ref); }
#pragma endregion

#pragma region funcs: numeric convert
__forceinline fn to_uint(i8  val) -> u8  { return u8(val); }
__forceinline fn to_uint(i16 val) -> u16 { return u16(val); }
__forceinline fn to_uint(i32 val) -> u32 { return u32(val); }
__forceinline fn to_uint(i64 val) -> u64 { return u64(val); }

__forceinline fn to_uint(u8  val) -> u8  { return val; }
__forceinline fn to_uint(u16 val) -> u16 { return val; }
__forceinline fn to_uint(u32 val) -> u32 { return val; }
__forceinline fn to_uint(u64 val) -> u64 { return val; }

__forceinline fn to_sint(i8  val) -> i8  { return val; }
__forceinline fn to_sint(i16 val) -> i16 { return val; }
__forceinline fn to_sint(i32 val) -> i32 { return val; }
__forceinline fn to_sint(i64 val) -> i64 { return val; }

__forceinline fn to_sint(u8  val) -> i8  { return i8(val); }
__forceinline fn to_sint(u16 val) -> i16 { return i16(val); }
__forceinline fn to_sint(u32 val) -> i32 { return i32(val); }
__forceinline fn to_sint(u64 val) -> i64 { return i64(val); }
#pragma endregion

#pragma region funcs: numeric
template<typename Ta>
constexpr fn sum(Ta a) {
    return a;
}

template<typename Ta, typename Tb, typename ...Tc>
constexpr fn sum(Ta a, Tb b, Tc... c) {
    return a + sum(b, c...);
}

template<typename Tv>
constexpr fn vsum($usize<>, const Tv& v) {
    return decltype(v[0])(0);
}

template<usize ...Idim, typename Tv>
constexpr fn vsum($usize<Idim...>, const Tv& v) {
    return sum(v[Idim]...);
}

template<typename Ta>
constexpr fn prod(Ta a) {
    return a;
}

template<typename Ta, typename Tb, typename ...Tc>
constexpr fn prod(Ta a, Tb b, Tc... c) {
    return a * prod(b, c...);
}

template<typename Tv>
constexpr fn vprod($usize<>, const Tv& v) {
    return decltype(v[0])(1);
}

template<usize ...Idim, typename Tv>
constexpr fn vprod($usize<Idim...>, const Tv& v) {
    return prod(v[Idim]...);
}

template<typename Ta>
constexpr fn (max)(Ta a) {
    return a;
}

template<typename Ta, typename Tb, typename ...Tc>
constexpr fn (max)(Ta a, Tb b, Tc... c) {
    return a > (max)(b, c...) ? a : (max)(b, c...);
}

template<typename Ta>
constexpr fn (min)(Ta a) {
    return a;
}
template<typename Ta, typename Tb, typename ...Tc>
constexpr fn (min)(Ta a, Tb b, Tc... c) {
    return a < (min)(b, c...) ? a : (min)(b, c...);
}
#pragma endregion

#pragma region $for
template<usize I, usize N, typename F>
fn _$for($usize<I>, $usize<N>, F&& f) -> void {
    f($usize<I>{});
    _$for($usize<I + 1>{}, $usize<N>{}, f);
}

template<usize N, typename F>
fn _$for($usize<N>, $usize<N>, F&& f) -> void {
    (void)f;
}

template<usize N, typename F>
fn $for(F&& f) -> void {
    _$for($usize<0>{}, $usize<N>{}, f);
}

#pragma endregion

}

inline void* operator new(ustd::usize size, void* address, ustd::$type<>) noexcept {
    (void)size;
    return address;
}

inline void operator delete(void*, void*, ustd::$type<>) noexcept {
    return;
}


namespace ustd
{

template<class T, typename ...U>
T* $new(T* address, U&& ...args) {
    return new((void*)address, $type<>{})T(as_fwd<U>(args)...);
}

}
