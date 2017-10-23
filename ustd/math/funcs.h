#pragma once

#include <ustd/core.h>

extern "C"
{
    using ustd::f32;
    using ustd::f64;

    pub fn logf  (f32 x) -> f32;            pub fn log  (f64 x) -> f64;    
    pub fn log10f(f32 x) -> f32;            pub fn log10(f64 x) -> f64;
    pub fn expf  (f32 x) -> f32;            pub fn exp  (f64 x) -> f64;
    pub fn sqrtf (f32 x) -> f32;            pub fn sqrt (f64 x) -> f64;

    pub fn sinf (f32 x) -> f32;             pub fn sin  (f64 x) -> f64;
    pub fn cosf (f32 x) -> f32;             pub fn cos  (f64 x) -> f64;
    pub fn tanf (f32 x) -> f32;             pub fn tan  (f64 x) -> f64;

    pub fn asinf(f32 x) -> f32;             pub fn asin (f64 x) -> f64;
    pub fn acosf(f32 x) -> f32;             pub fn acos (f64 x) -> f64;
    pub fn atanf(f32 x) -> f32;             pub fn atan (f64 x) -> f64;

    pub fn powf  (f32 x, f32 y) -> f32;     pub fn pow  (f64 x, f64 y) -> f64;
    pub fn atan2f(f32 y, f32 x) -> f32;     pub fn atan2(f64 y, f64 x) -> f64;
}

namespace ustd::math
{

template<class T >
constexpr fn (max)(T a, T b) noexcept -> T { return a > b ? a : b; }

template<class T >
constexpr fn (min)(T a, T b) noexcept -> T { return a < b ? a : b; }

template<class T>
constexpr fn (abs)(T x) noexcept -> T { return x < 0 ? -x : x; }

inline fn exp(f32 x) noexcept -> f32 { return ::expf(x); }
inline fn exp(f64 x) noexcept -> f64 { return ::exp(x); }

inline fn pow(f32 x, f32 y) noexcept -> f32 { return ::powf(x, y); }
inline fn pow(f64 x, f64 y) noexcept -> f64 { return ::pow(x, y); }

inline fn ln(f32 x) noexcept -> f32 { return ::logf(x); }
inline fn ln(f64 x) noexcept -> f64 { return ::log(x); }

inline fn log10(f32 x) noexcept -> f32 { return ::log10f(x); }
inline fn log10(f64 x) noexcept -> f64 { return ::log10(x); }

inline fn sin(f32 x) noexcept -> f32 { return ::sinf(x); }
inline fn sin(f64 x) noexcept -> f64 { return ::sin(x); }

inline fn cos(f32 x) noexcept -> f32 { return ::cosf(x); }
inline fn cos(f64 x) noexcept -> f64 { return ::cos(x); }

inline fn tan(f32 x) noexcept -> f32 { return ::tanf(x); }
inline fn tan(f64 x) noexcept -> f64 { return ::tan(x); }

inline fn asin(f32 x) noexcept -> f32 { return ::asinf(x); }
inline fn asin(f64 x) noexcept -> f64 { return ::asin(x); }

inline fn acos(f32 x) noexcept -> f32 { return ::acosf(x); }
inline fn acos(f64 x) noexcept -> f64 { return ::acos(x); }

inline fn atan(f32 x) { return ::atanf(x); }
inline fn atan(f64 x) { return ::atan(x); }

inline fn atan2(f32 x, f32 y) { return ::atan2f(x, y); }
inline fn atan2(f64 x, f64 y) { return ::atan2(x, y); }

}