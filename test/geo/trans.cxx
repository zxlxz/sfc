#include "sfc/geo.h"
#include "sfc/test.h"

namespace sfc::geo {

SFC_TEST(trans_zoom) {
  const auto t = Affine{}.zoom(2);

  // vec
  const auto v0 = Vector{1, 1};
  const auto v1 = t(v0);
  test::assert_flt_eq(v1.x(), 2);
  test::assert_flt_eq(v1.y(), 2);

  // point
  const auto p0 = Point{1, 1};
  const auto p1 = t(p0);
  test::assert_flt_eq(p1.x(), 2);
  test::assert_flt_eq(p1.y(), 2);
}

SFC_TEST(trans_shift) {
  const auto t = Affine{}.translate({2, 1});

  // vec
  const auto v0 = Vector{1, 1};
  const auto v1 = t(v0);
  test::assert_flt_eq(v1.x(), 1);
  test::assert_flt_eq(v1.y(), 1);

  // point
  const auto p0 = Point{1, 1};
  const auto p1 = t(p0);
  test::assert_flt_eq(p1.x(), 3);
  test::assert_flt_eq(p1.y(), 2);
}

SFC_TEST(trans_rotate) {
  const auto t = Affine{}.rotate(Angle::from_deg(90));

  // vec
  const auto v0 = Vector{1, 1};
  const auto v1 = t(v0);
  test::assert_flt_eq(v1.x(), -1);
  test::assert_flt_eq(v1.y(), +1);

  // point
  const auto p0 = Point{1, 1};
  const auto p1 = t(p0);
  test::assert_flt_eq(p1.x(), -1);
  test::assert_flt_eq(p1.y(), +1);
}

SFC_TEST(trans_affine) {
  auto p = Point{1, 0};

  const auto t = Affine{}.zoom(100).rotate(Angle::from_deg(90)).translate({100, 100});

  const auto a = t(p);
  test::assert_eq(a, Point(100, 200));

  auto b = t.inv()(a);
  test::assert_eq(p, b);
}

}  // namespace sfc::geo
