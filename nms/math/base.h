#pragma once

#include <nms/core.h>

namespace nms::math
{

/* max */
template<class T, class=$when_is<$number, T> >
constexpr T max(T a, T b) { return a > b ? a : b; }

template<class T, class=$when_is<$number, T> >
constexpr T min(T a, T b) { return a < b ? a : b; }

/* abs */
constexpr i8  abs(i8  val) { return val < 0 ? -val : val; }
constexpr i16 abs(i16 val) { return val < 0 ? -val : val; }
constexpr i32 abs(i32 val) { return val < 0 ? -val : val; }
constexpr i64 abs(i64 val) { return val < 0 ? -val : val; }
constexpr f32 abs(f32 val) { return val < 0 ? -val : val; }
constexpr f64 abs(f64 val) { return val < 0 ? -val : val; }

/* isnan */
template<class T>
inline auto (isnan)(const T& value) {
#ifdef NMS_CC_MSVC
    return ::isnan(value);
#else
    return isnan(value);
#endif
}
#ifndef isnan
#undef isnan
#endif

/* isinf */
template<class T>
inline auto (isinf)(const T& value) {
#ifdef NMS_CC_MSVC
    return ::isinf(value);
#else
    return isinf(value);
#endif
}
#ifdef isinf
#undef isinf
#endif

inline f32 exp(f32 val)     { return ::expf(val); }
inline f64 exp(f64 val)     { return ::exp (val); }

inline f32 pow(f32 x, f32 y) { return ::powf(x, y); }
inline f64 pow(f64 x, f64 y) { return ::pow (x, y); }

/* caculate natural logarithm */
inline f32 ln(f32 val) { return ::logf(val); }
inline f64 ln(f64 val) { return ::log (val); }

/* caculate the base 10 logarithm */
inline f32 log10(f32 val) { return ::log10f(val); }
inline f64 log10(f64 val) { return ::log10 (val); }

/* sin/cos/tan */
inline f32 sin(f32 val) { return ::sinf(val); }
inline f64 sin(f64 val) { return ::sin (val); }

inline f32 cos(f32 val) { return ::cosf(val); }
inline f64 cos(f64 val) { return ::cos (val); }

inline f32 tan(f32 val) { return ::tanf(val); }
inline f64 tan(f64 val) { return ::tan (val); }

/* asin/acos/atan */
inline f32 asin(f32 val) { return ::asinf(val); }
inline f64 asin(f64 val) { return ::asin (val); }

inline f32 acos(f32 val) { return ::acosf(val); }
inline f64 acos(f64 val) { return ::acos (val); }

inline f32 atan(f32 val) { return ::atanf(val); }
inline f64 atan(f64 val) { return ::atan (val); }

/* atan2 */
inline f32 atan2(f32 x, f32 y) { return ::atan2f(x, y); }
inline f64 atan2(f64 x, f64 y) { return ::atan2(x, y); }

// [min]
struct Min
{
    template<class T> __forceinline static auto run(T x, T y)  noexcept { return x < y ? x : y; }
};

// [max]
struct Max
{
    template<class T> __forceinline static auto run(T x, T y)  noexcept { return x > y ? x : y; }
};

#pragma region logic
struct Eq  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x == y; } };
struct Neq { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x != y; } };
struct Lt  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x <  y; } };
struct Gt  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x >  y; } };
struct Le  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x <= y; } };
struct Ge  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x >= y; } };
struct And { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x && y; } };
struct Or  { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x || y; } };
#pragma endregion

struct Pos { template<class T> __forceinline static auto run(T t) noexcept { return +t; } };
struct Neg { template<class T> __forceinline static auto run(T t) noexcept { return -t; } };
struct Abs { template<class T> __forceinline static auto run(T t) noexcept { return t >= 0 ? +t : -t; } };

struct Add { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x + y; } };
struct Sub { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x - y; } };
struct Mul { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x * y; } };
struct Div { template<class X, class Y> __forceinline static auto run(X x, Y y) noexcept { return x / y; } };

// [pow2, sqrt, exp, ln, log10](t)
struct Pow2    { template<class T> __forceinline static auto run(T t) noexcept { return t*t;       } };
struct Sqrt    { template<class T> __forceinline static auto run(T t) noexcept { return sqrt(t);   } };
struct Exp     { template<class T> __forceinline static auto run(T t) noexcept { return exp(t);    } };
struct Ln      { template<class T> __forceinline static auto run(T t) noexcept { return ln(t);     } };
struct Log10   { template<class T> __forceinline static auto run(T t) noexcept { return log10(t);  } };

// [sin,cos,tan](t)
struct Sin      { template<class T> __forceinline static auto run(T t) noexcept { return sin(t);    } };
struct Cos      { template<class T> __forceinline static auto run(T t) noexcept { return cos(t);    } };
struct Tan      { template<class T> __forceinline static auto run(T t) noexcept { return tan(t);    } };

// [asin, acos, atan](t)
struct Asin     { template<class T> __forceinline static auto run(T t) noexcept { return asin(t);   } };
struct Acos     { template<class T> __forceinline static auto run(T t) noexcept { return acos(t);   } };
struct Atan     { template<class T> __forceinline static auto run(T t) noexcept { return atan(t);   } };

// [pow](x, y)
struct Pow      { template<class T>__forceinline static auto run(T x, T y) noexcept { return pow(x, y); } };

// [+-*/]=
struct Ass2 { template<class X, class Y> __forceinline static auto& run(Y& y, X x) noexcept { return y = x; } };
struct Add2 { template<class X, class Y> __forceinline static auto& run(Y& y, X x) noexcept { return y += x; } };
struct Sub2 { template<class X, class Y> __forceinline static auto& run(Y& y, X x) noexcept { return y -= x; } };
struct Mul2 { template<class X, class Y> __forceinline static auto& run(Y& y, X x) noexcept { return y *= x; } };
struct Div2 { template<class X, class Y> __forceinline static auto& run(Y& y, X x) noexcept { return y /= x; } };

}
