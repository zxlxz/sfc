#pragma once

#include "vector.h"

namespace sfc::geo {

struct Point {
  f64 _x = 0.0;
  f64 _y = 0.0;

 public:
  [[sfc_inline]] Point() noexcept = default;

  [[sfc_inline]] Point(f64 x, f64 y) : _x{x}, _y{y} {}

  [[sfc_inline]] static auto from(const auto& p) -> Point {
    return Point{p.x, p.y};
  }

  [[sfc_inline]] auto x() const -> f64 {
    return _x;
  }

  [[sfc_inline]] auto y() const -> f64 {
    return _y;
  }

  [[sfc_inline]] auto as_vec() const -> Vector {
    return Vector{_x, _y};
  }

  [[sfc_inline]] auto operator==(const Point& other) const -> bool {
    return _x == other._x && _y == other._y;
  }

  // B = A + v
  [[sfc_inline]] auto operator+(const Vector& vec) const -> Point {
    return Point{_x + vec._x, _y + vec._y};
  }

  // B = A - v
  [[sfc_inline]] auto operator-(const Vector& vec) const -> Point {
    return Point{_x - vec._x, _y - vec._y};
  }

  // v = A -> B
  auto operator-(const Point& other) const -> Vector {
    return {_x - other._x, _y - other._y};
  }

  // |A-B|
  [[sfc_inline]] auto distance_to(const Point& point) const -> f64 {
    return (point - *this).length();
  }

  // k*P
  [[sfc_inline]] friend auto operator*(f64 k, const Point& p) -> Point {
    return Point{k * p._x, k * p._y};
  }

  void fmt(auto& f) const {
    auto imp = f.debug_struct();
    imp.field("x", _x);
    imp.field("y", _y);
  }
};

[[sfc_inline]] inline auto Vector::as_point() const -> Point {
  return Point{_x, _y};
}

}  // namespace sfc::geo
