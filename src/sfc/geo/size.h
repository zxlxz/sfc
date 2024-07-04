#pragma once

#include "point.h"

namespace sfc::geo {

struct Size {
  f64 _length = 0.0;
  f64 _width = 0.0;

 public:
  Size() noexcept = default;

  Size(f64 length, f64 width) : _length{cmp::max(0.0, length)}, _width{cmp::max(0.0, width)} {}

  auto length() const -> f64 {
    return _length;
  }

  auto width() const -> f64 {
    return _width;
  }

  auto area() const -> f64 {
    return _length * _width;
  }

  auto contains(const Vector& vector) const -> bool {
    const auto dx = math::fabs(vector._x) - _length / 2.0;
    const auto dy = math::fabs(vector._y) - _width / 2.0;
    return dx < 0 || dy < 0;
  }

  auto contains(const Point& point) const -> bool {
    const auto dx = math::fabs(point._x) - _length / 2.0;
    const auto dy = math::fabs(point._y) - _width / 2.0;
    return dx < 0 || dy < 0;
  }

  auto operator+(const Size& other) const -> Size {
    const auto l = _length + other._length;
    const auto w = _width + other._width;
    return Size{l, w};
  }

  auto operator-(const Size& other) const -> Size {
    const auto l = _length - other._length;
    const auto w = _width - other._width;
    return Size{l, w};
  }

  auto operator/(f64 scale) const -> Size {
    return Size{_length / scale, _width / scale};
  }

  friend auto operator*(f64 scale, const Size& size) -> Size {
    return Size{scale * size._length, scale * size._width};
  }

  auto distance_to(const Point& point) const -> f64 {
    const auto dx = math::fabs(point._x) - _length / 2.0;
    const auto dy = math::fabs(point._y) - _width / 2.0;
    if (dx <= 0) return dy <= 0 ? 0 : dy;
    if (dy <= 0) return dx;
    return math::hypot(dx, dy);
  }

  void fmt(auto& f) const {
    auto imp = f.debug_struct();
    imp.field("length", _length);
    imp.field("width", _width);
  }
};

}  // namespace sfc::geo
