#pragma once

#include "aabox.h"

namespace sfc::geo {

class PointCloud {
  Vec<Point> _points = {};

 public:
  PointCloud() noexcept = default;

  explicit PointCloud(Vec<Point> points) noexcept : _points{mem::move(points)} {}

  ~PointCloud() = default;

  PointCloud(PointCloud&&) noexcept = default;

  PointCloud& operator=(PointCloud&&) noexcept = default;

  static auto from(auto&& points) -> PointCloud {
    return PointCloud{Vec<Point>::from(points)};
  }

  auto clone() const -> PointCloud {
    return PointCloud{_points.clone()};
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

  auto points() const -> Slice<const Point> {
    return _points.as_slice();
  }

  void push(Point point) {
    _points.push(point);
  }

  void reserve(usize additional) {
    _points.reserve(additional);
  }

  void fmt(auto&& f) const {
    _points.fmt(f);
  }

  auto transform(const auto& f) const -> PointCloud {
    auto res = PointCloud{};
    res._points.reserve(_points.len());

    for (const auto& p : _points.as_slice()) {
      const auto q = f(p);
      res._points.push(q);
    }
    return res;
  }

  auto filt(auto&& f) const -> PointCloud {
    auto res = PointCloud{};
    _points.iter().for_each([&](const auto& p) {
      if (f(p)) res._points.push(p);
    });
    return res;
  }

  auto bounding_box() const -> AABox {
    return AABox::from_points(_points);
  }
};

}  // namespace sfc::geo
