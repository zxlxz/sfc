#pragma once

#include "sfc/math.h"

namespace sfc::geo {

static constexpr f64 EPSILON = 1e-10;

struct Vector {
  f64 _x = 0;
  f64 _y = 0;

 public:
  [[sfc_inline]] Vector() = default;

  [[sfc_inline]] Vector(f64 x, f64 y) : _x{x}, _y{y} {}

  [[sfc_inline]] auto x() const -> f64 {
    return _x;
  }

  [[sfc_inline]] auto y() const -> f64 {
    return _y;
  }

  // (x, -y)
  [[sfc_inline]] auto conj() const -> Vector {
    return Vector{_x, -_y};
  }

  // sqrt(x^2+y^2)
  [[sfc_inline]] auto length() const -> f64 {
    return math::hypot(_x, _y);
  }

  // x^2+y^2
  [[sfc_inline]] auto length_sqr() const -> f64 {
    return _x * _x + _y * _y;
  }

  // |x|+|y|
  [[sfc_inline]] auto norm1() const -> f64 {
    return math::fabs(_x) + math::fabs(_y);
  }

  // sqrt(x^2+y2)
  [[sfc_inline]] auto norm2() const -> f64 {
    return math::hypot(_x, _y);
  }

  // unit vector
  [[sfc_inline]] auto to_unit() const -> Vector {
    const auto s = this->length();
    if (s < EPSILON) {
      return Vector{1.0, 0.0};
    }
    return Vector{_x / s, _y / s};
  }

  auto as_point() const -> struct Point;

  auto to_angle() const -> struct Angle;

  [[sfc_inline]] auto operator*(const Vector& rhs) const -> Vector {
    const auto x = _x * rhs._x - _y * rhs._y;
    const auto y = _x * rhs._y + _y * rhs._x;
    return Vector{x, y};
  }

  [[sfc_inline]] auto operator==(const Vector& other) const -> bool {
    return _x == other._x && _y == other._y;
  }

  [[sfc_inline]] auto operator!=(const Vector& other) const -> bool {
    return _x != other._x || _y != other._y;
  }

  [[sfc_inline]] auto operator-() const -> Vector {
    return Vector{-_x, -_y};
  }

  [[sfc_inline]] auto operator+(Vector v) const -> Vector {
    return Vector{_x + v._x, _y + v._y};
  }

  [[sfc_inline]] auto operator-(Vector v) const -> Vector {
    return Vector{_x - v._x, _y - v._y};
  }

  [[sfc_inline]] auto operator/(f64 k) const -> Vector {
    return Vector{_x / k, _y / k};
  }

  [[sfc_inline]] auto scale(f64 k) const -> Vector {
    return Vector{k * _x, k * _y};
  }

  [[sfc_inline]] auto dot_prod(const Vector& other) const -> f64 {
    return _x * other._x + _y * other._y;
  }

  [[sfc_inline]] auto cross_prod(const Vector& other) const -> f64 {
    return _x * other._y - _y * other._x;
  }

  // scalar prod
  [[sfc_inline]] friend auto operator*(f64 k, const Vector& v) -> Vector {
    return v.scale(k);
  }

  void fmt(auto& f) const {
    auto imp = f.debug_list();
    imp.entry(_x);
    imp.entry(_y);
  }
};

}  // namespace sfc::geo
