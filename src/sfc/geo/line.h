#pragma once

#include "angle.h"
#include "point.h"

namespace sfc::geo {

struct Line {
  Point _a{};
  Point _b{};

 public:
  [[sfc_inline]] Line() = default;

  [[sfc_inline]] Line(Point a, Point b) : _a{a}, _b{b} {}

  [[sfc_inline]] auto as_vec() const -> Vector {
    return _b - _a;
  }

  auto contains(const Point& point) const -> bool;
  auto distance_to(const Point& point) const -> f64;

  auto has_interset(const Line& line) const -> bool;
  auto get_intersect(const Line& line) const -> Point;
};

struct LineSegment {
  Point _a{};
  Point _b{};

 public:
  [[sfc_inline]] LineSegment() = default;

  [[sfc_inline]] LineSegment(Point start, Point end) : _a{start}, _b{end} {}

  [[sfc_inline]] auto a() const -> Point {
    return _a;
  }

  [[sfc_inline]] auto b() const -> Point {
    return _b;
  }

  [[sfc_inline]] auto as_vec() const -> Vector {
    return _b - _a;
  }

  [[sfc_inline]] auto length() const -> f64 {
    return this->as_vec().length();
  }

  [[sfc_inline]] auto center() const -> Point {
    return Point{0.5 * (_a._x + _b._x), 0.5 * (_a._y + _b._y)};
  }

  [[sfc_inline]] auto get_interp_point(f64 frac) const -> Point {
    if (frac <= 0) return _a;
    if (frac >= 1) return _b;
    return _a + frac * (_b - _a);
  }

  [[sfc_inline]] auto as_line() const -> Line {
    return Line{_a, _b};
  }

  auto contains(const Point& point) const -> bool;

  auto has_interset(const LineSegment& line) const -> bool;

  auto get_intersect(const LineSegment& line) const -> Point;

  auto transform(const auto& f) const -> LineSegment {
    auto res = LineSegment{f(_a), f(_b)};
    return res;
  }

  void fmt(auto& f) const {
    auto imp = f.debug_struct();
    imp.field("a", _a);
    imp.field("b", _b);
  }
};

}  // namespace sfc::geo
