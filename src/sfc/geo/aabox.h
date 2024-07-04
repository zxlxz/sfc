#pragma once

#include "size.h"

namespace sfc::geo {

struct AABox {
  f64 _x_min;
  f64 _x_max;
  f64 _y_min;
  f64 _y_max;

 public:
  AABox() = default;

  AABox(const f64 (&x_limit)[2], const f64 (&y_limit)[2])
      : _x_min{x_limit[0]}, _x_max{x_limit[1]}, _y_min{y_limit[0]}, _y_max{y_limit[1]} {}

  static auto with_size_center(Size size, Point center) -> AABox;

  static auto from_points(const auto& points) -> AABox {
    auto res = AABox{};
    points.iter().for_each([&](const auto& p) { res.merge_point(p); });
    return res;
  }

  auto size() const -> Size {
    return Size{_x_max - _x_min, _y_max - _y_min};
  }

  auto center() const -> Point {
    return Point{(_x_min + _x_max) / 2, (_y_min + _y_max) / 2};
  }

  // X-axis
  [[sfc_inline]] auto length() const -> double {
    return _x_max - _x_min;
  }

  // Y-axis
  [[sfc_inline]] auto width() const -> double {
    return _y_max - _y_min;
  }

  [[sfc_inline]] auto x_min() const -> double {
    return _x_min;
  }

  [[sfc_inline]] auto x_max() const -> double {
    return _x_max;
  }

  [[sfc_inline]] auto y_min() const -> double {
    return _y_min;
  }

  [[sfc_inline]] auto y_max() const -> double {
    return _y_max;
  }

  [[sfc_inline]] void merge_x(f64 x) {
    if (x < _x_min) _x_min = x;
    if (x > _x_max) _x_max = x;
  }

  [[sfc_inline]] void merge_y(f64 y) {
    if (y < _y_min) _y_min = y;
    if (y > _y_max) _y_max = y;
  }

  [[sfc_inline]] auto contains_x(f64 x) const -> bool {
    return _x_min <= x && x < _x_max;
  }

  [[sfc_inline]] auto contains_y(f64 y) const -> bool {
    return _y_min <= y && y < _y_max;
  }

  auto contains(const Point& point) const -> bool {
    return this->contains_x(point._x) && this->contains_y(point._y);
  }

  void merge_point(const Point& point) {
    this->merge_x(point._x);
    this->merge_x(point._y);
  }

  auto has_overlap(const AABox& other) const -> bool;

  void merge_rect(const AABox& other);

  // intersection
  auto operator&(const AABox& other) const -> AABox;

  // union
  auto operator|(const AABox& other) const -> AABox;
};

}  // namespace sfc::geo
