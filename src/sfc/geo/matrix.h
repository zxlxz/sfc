#pragma once

#include "vector.h"

namespace sfc::geo {

struct Mat2x2 {
  f64 _a = 0;
  f64 _b = 0;
  f64 _c = 0;
  f64 _d = 0;

 public:
  Mat2x2() = default;

  Mat2x2(const f64 (&r0)[2], const f64 (&r1)[2]) : _a{r0[0]}, _b{r0[1]}, _c{r1[0]}, _d{r1[1]} {}

  static auto eye() -> Mat2x2 {
    return Mat2x2{{1, 0}, {0, 1}};
  }

  auto det() const -> f64 {
    return _a * _d - _b * _c;
  }

  auto inv() const -> Mat2x2 {
    const auto k = 1.0 / this->det();
    const auto m = Mat2x2{{k * _d, -k * _b}, {-k * _c, k * _a}};
    return m;
  }

  // A*v
  [[sfc_inline]] auto operator*(const Vector& v) const -> Vector {
    const auto x = _a * v._x + _b * v._y;
    const auto y = _c * v._x + _d * v._y;
    return Vector{x, y};
  }

  // A*B
  [[sfc_inline]] auto operator*(const Mat2x2& m) const -> Mat2x2 {
    const auto a = *this * Vector{m._a, m._c};
    const auto b = *this * Vector{m._b, m._d};
    return Mat2x2{{a._x, b._x}, {a._y, b._y}};
  }

  // kA
  [[sfc_inline]] friend auto operator*(f64 k, const Mat2x2& m) -> Mat2x2 {
    return Mat2x2{{k * m._a, k * m._b}, {k * m._c, k * m._d}};
  }

  void fmt(auto& f) const {
    auto imp = f.list();
    imp.entry(Vector{_a, _b});
    imp.entry(Vector{_c, _d});
  }
};

}  // namespace sfc::geo
