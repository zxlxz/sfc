#include "line.h"

namespace sfc::geo {

namespace detail {
static auto is_within(double p, double a, double b) {
  if (a > b) {
    mem::swap(a, b);
  }
  return p >= a && p <= b;
}
}  // namespace detail

auto Line::contains(const Point& p) const -> bool {
  const auto v = this->as_vec().to_unit();
  const auto u = p - _a;

  const auto cross_prod = u.cross_prod(v);
  return cross_prod < EPSILON;
}

auto Line::distance_to(const Point& p) const -> f64 {
  const auto v = this->as_vec().to_unit();
  const auto u = p - _a;

  const auto cross_prod = u.cross_prod(v);
  return math::fabs(cross_prod);
}

auto Line::has_interset(const Line& line) const -> bool {
  const auto& l1 = *this;
  const auto& l2 = line;

  const auto v = l1.as_vec().to_unit();
  const auto u = l2.as_vec().to_unit();
  const auto d = v.cross_prod(u);
  return math::fabs(d) > EPSILON;
}

auto Line::get_intersect(const Line& line) const -> Point {
  const auto& l1 = *this;
  const auto& l2 = line;

  const auto v1 = l1.as_vec();
  const auto v2 = l2.as_vec();
  const auto c1 = v1.cross_prod(l2._a - l1._a);  // PA
  const auto c2 = v1.cross_prod(l2._b - l1._a);  // PB
  if (math::fabs(c2 - c1) < EPSILON) {
    return Point{num::nan(), num::nan()};
  }

  const auto ratio = -c1 / (c2 - c1);
  const auto point = l2._a + ratio * v2;
  return point;
}

auto LineSegment::contains(const Point& p) const -> bool {
  if ((p - _a).norm1() < EPSILON) {
    return true;
  }
  if ((p - _b).norm1() < EPSILON) {
    return true;
  }

  const auto u = p - _a;
  const auto d = u.cross_prod(_b - _a);
  if (math::fabs(d) > EPSILON) {
    return false;
  }
  const auto ret = detail::is_within(p._x, _a._x, _b._x) && detail::is_within(p._y, _a._y, _b._y);

  return ret;
}

auto LineSegment::has_interset(const LineSegment& line) const -> bool {
  const auto& l1 = *this;
  const auto& l2 = line;

  const auto v1 = l1.as_vec();
  const auto v2 = l2.as_vec();

  const auto cc1 = v1.cross_prod(l2._a - l1._a);
  const auto cc2 = v1.cross_prod(l2._b - l1._a);
  if (cc1 * cc2 > 0) {
    return false;
  }

  const auto cc3 = v2.cross_prod(l1._a - l2._a);
  const auto cc4 = v2.cross_prod(l1._b - l2._a);
  if (cc3 * cc4 > 0) {
    return false;
  }

  return true;
}

auto LineSegment::get_intersect(const LineSegment& line) const -> Point {
  // check endpoint on line?
  if (this->contains(line._a)) {
    return line._a;
  }
  if (this->contains(line._b)) {
    return line._b;
  }
  if (line.contains(_a)) {
    return _a;
  }
  if (line.contains(_b)) {
    return _b;
  }

  // to short!
  if (this->as_vec().norm1() < EPSILON || line.as_vec().norm1() < EPSILON) {
    return Point{num::nan(), num::nan()};
  }

  // cross prod, caculate interset position
  const auto v1 = this->as_vec();
  const auto c1 = v1.cross_prod(line._a - _a);
  const auto c2 = v1.cross_prod(line._b - _a);
  if (c1 * c2 >= -EPSILON) {
    return Point{num::nan(), num::nan()};
  }

  // cross prod, caculate interset position
  const auto v2 = line.as_vec();
  const auto c3 = v2.cross_prod(_a - line._a);
  const auto c4 = v2.cross_prod(_b - line._a);
  if (c3 * c4 >= -EPSILON) {
    return Point{num::nan(), num::nan()};
  }

  // now, caculate interset point
  const auto ratio = -c3 / (c4 - c3);
  const auto point = _a + ratio * v1;
  return point;
}

}  // namespace sfc::geo
