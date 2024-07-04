#pragma once

#include "point.h"

namespace sfc::geo {

struct Ray {
  Point _point = {0, 0};
  Vector _vector = {0, 0};

 public:
  [[sfc_inline]] Ray() = default;

  [[sfc_inline]] Ray(Point point, Vector vec) : _point{point}, _vector{vec} {}

  [[sfc_inline]] auto point() const -> const Point& {
    return _point;
  }

  [[sfc_inline]] auto vector() const -> const Vector& {
    return _vector;
  }

  void fmt(auto& f) const {
    auto imp = f.object();
    imp.field("point", _point);
    imp.field("vector", _vector);
  }
};

}  // namespace sfc::geo
