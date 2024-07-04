#pragma once

#include "point_cloud.h"

namespace sfc::geo {

class Polygon {
  Vec<Point> _points = {};

 public:
  Polygon() noexcept = default;

  Polygon(Vec<Point> points) noexcept : _points{mem::move(points)} {}

  ~Polygon() = default;

  Polygon(Polygon&&) noexcept = default;

  Polygon& operator=(Polygon&&) noexcept = default;

  auto clone() const -> Polygon {
    return Polygon{_points.clone()};
  };

  static auto from_vertexs(auto& points) -> Polygon {
    return Polygon{Vec<Point>::from(points)};
  }

  operator bool() const {
    return _points;
  }

 public:
  auto points() const -> Slice<const Point> {
    return _points.as_slice();
  }

  auto len() const -> usize {
    return _points.len();
  }

  auto operator[](usize idx) const -> const Point& {
    return _points[idx];
  }

  auto operator[](usize idx) -> Point& {
    return _points[idx];
  }

 public:
  void push(Point point) {
    _points.push(point);
  }

  void reserve(usize additional) {
    _points.reserve(additional);
  }

  void fmt(auto& f) const {
    _points.fmt(f);
  }

 public:
  auto vertex_order() const -> int;

  auto is_convex() const -> bool;

  auto area() const -> double;

  auto centroid() const -> Point;

  auto bounding_box() const -> AABox {
    auto res = AABox{};
    _points.iter().for_each([&](const auto& p) { res.merge_point(p); });
    return res;
  }

  auto transform(const auto& f) const -> Polygon {
    auto res = Polygon{};
    res._points.reserve(_points.len());

    for (const auto& p : _points.as_slice()) {
      const auto q = f(p);
      res._points.push(q);
    }
    return res;
  }
};

}  // namespace sfc::geo
