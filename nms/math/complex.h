#pragma once

#include <nms/core/type.h>

namespace nms::math
{

template<class T>
struct complex
{
    union { T real; T r; };
    union { T imag; T i; };

public:
    constexpr complex() = default;
    constexpr complex(const T& r)               : real(r), imag(0) {}
    constexpr complex(const T& r, const T& i)   : real(r), imag(i) {}
};
using cf32 = complex<f32>;
using cf64 = complex<f64>;

template<class T> constexpr T abs(complex<T> t) { return t.r*t.r + t.i*t.i; }

template<class T> constexpr complex<T> operator~(complex<T> t) { return { t.r, -t.i }; }

template<class T> constexpr complex<T> operator+(complex<T> a, T b) { return { a.r + b, a.i }; }
template<class T> constexpr complex<T> operator-(complex<T> a, T b) { return { a.r - b, a.i }; }
template<class T> constexpr complex<T> operator*(complex<T> a, T b) { return { a.r * b, a.i }; }
template<class T> constexpr complex<T> operator/(complex<T> a, T b) { return { a.r / b, a.i }; }

template<class T> constexpr complex<T> operator+(T a, complex<T> b) { return { a + b.r, b.i }; }
template<class T> constexpr complex<T> operator-(T a, complex<T> b) { return { a - b.r, b.i }; }
template<class T> constexpr complex<T> operator*(T a, complex<T> b) { return { a * b.r, a * b.i }; }
template<class T> constexpr complex<T> operator/(T a, complex<T> b) { return { a * b.r/(b.r*b.r+b.i*b.i), a * b.i/(b.r*b.r+b.i*b.i)}; }

template<class T> constexpr complex<T> operator+(complex<T> a, complex<T> b) { return { a.r + b.r, a.i + b.i }; }
template<class T> constexpr complex<T> operator-(complex<T> a, complex<T> b) { return { a.r - b.r, a.i - b.i }; }
template<class T> constexpr complex<T> operator*(complex<T> a, complex<T> b) { return { a.r * b.r-a.i*b.i, a.r*b.i + a.i*b.r }; }
template<class T> constexpr complex<T> operator/(complex<T> a, complex<T> b) { return { (a*~b) / abs(b) }; }

template<class T, class U> complex<T> operator+=(complex<T>& a, U b) { a = a + b;  return a; }
template<class T, class U> complex<T> operator-=(complex<T>& a, U b) { a = a - b;  return a; }
template<class T, class U> complex<T> operator*=(complex<T>& a, U b) { a = a * b;  return a; }
template<class T, class U> complex<T> operator/=(complex<T>& a, U b) { a = a / b;  return a; }

}

