#include "sfc/geo.h"
#include "sfc/test.h"

namespace sfc::geo {

SFC_TEST(line_as_vec) {
  const auto l = Line{{0, 1}, {1, -1}};
  const auto v = l.as_vec();
  test::assert_flt_eq(v.x(), 1);
  test::assert_flt_eq(v.y(), -2);
}

SFC_TEST(line_contains) {
  const auto l = Line{{-1, -2}, {1, 2}};
  test::assert_true(l.contains({0, 0}));
  test::assert_true(l.contains({4, 8}));
  test::assert_false(l.contains({4, -8}));
}

SFC_TEST(line_distance_to) {
  {
    const auto l = Line{{1, 0}, {1, 2}};
    test::assert_flt_eq(l.distance_to({0, 0}), 1);
    test::assert_flt_eq(l.distance_to({1, 1}), 0);
  }
  {
    const auto l = Line{{-1, -1}, {2, 2}};
    test::assert_flt_eq(l.distance_to({0, 0}), 0);
    test::assert_flt_eq(l.distance_to({2, 0}), math::sqrt(2.0));
  }
}

SFC_TEST(line_has_interset) {
  const auto l1 = Line{{0, 0}, {1, 1}};
  const auto l2 = Line{{1, 0}, {2, 0}};
  const auto l3 = Line{{3, 1}, {6, 1}};

  test::assert_true(l1.has_interset(l2));
  test::assert_true(l2.has_interset(l1));

  test::assert_true(l1.has_interset(l3));
  test::assert_true(l3.has_interset(l1));

  test::assert_false(l2.has_interset(l3));
  test::assert_false(l3.has_interset(l2));
}

SFC_TEST(line_get_intersect) {
  const auto l1 = Line{{0, 0}, {1, 1}};
  const auto l2 = Line{{1, 0}, {1, 1}};
  const auto l3 = Line{{2, 0}, {2, 1}};

  const auto a12 = l1.get_intersect(l2);
  const auto a13 = l1.get_intersect(l3);
  const auto a21 = l2.get_intersect(l1);
  const auto a23 = l2.get_intersect(l3);
  const auto a31 = l3.get_intersect(l1);
  const auto a32 = l3.get_intersect(l2);

  test::assert_eq(a12, a21);
  test::assert_eq(a13, a31);

  test::assert_flt_eq(a12._x, 1.0);
  test::assert_flt_eq(a12._y, 1.0);

  test::assert_true(num::isnan(a23._x) && num::isnan(a23._y));
  test::assert_true(num::isnan(a32._x) && num::isnan(a32._y));
}

SFC_TEST(line_segment_length) {
  const auto l1 = LineSegment{{1, 1}, {1, 1}};
  test::assert_flt_eq(l1.length(), 0);

  const auto l2 = LineSegment{{0, 0}, {1, 1}};
  test::assert_flt_eq(l2.length(), math::sqrt(2.0));
}

SFC_TEST(line_segment_center) {
  const auto l1 = LineSegment{{1, 0}, {3, 0}};
  const auto c1 = l1.center();
  test::assert_flt_eq(c1.x(), 2);
  test::assert_flt_eq(c1.y(), 0);
}

SFC_TEST(line_segment_get_interp_point) {
  const auto l1 = LineSegment{{0, 0}, {1, 0}};

  test::assert_eq(l1.get_interp_point(0).x(), 0.0);
  test::assert_eq(l1.get_interp_point(1).x(), 1.0);

  test::assert_eq(l1.get_interp_point(0.5).x(), 0.5);

  test::assert_eq(l1.get_interp_point(-1).x(), 0.0);
  test::assert_eq(l1.get_interp_point(2).x(), 1.0);
}

SFC_TEST(line_segment_has_interset) {
  const auto l1 = LineSegment{{0, 0}, {2, 2}};
  const auto l2 = LineSegment{{1, 0}, {1, 1}};
  const auto l3 = LineSegment{{1, 0}, {1, -1}};

  test::assert_true(l1.has_interset(l2));
  test::assert_false(l1.has_interset(l3));
}

SFC_TEST(line_segment_get_intersect) {
  const auto l1 = LineSegment{{0, 0}, {2, 2}};
  const auto l2 = LineSegment{{1, 0}, {1, 1}};
  const auto l3 = LineSegment{{1, 0}, {1, -1}};

  test::assert_flt_eq(l1.get_intersect(l2)._x, 1);
  test::assert_true(num::isnan(l1.get_intersect(l3)._x));
}

}  // namespace sfc::geo
