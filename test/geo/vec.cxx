#include "sfc/geo.h"
#include "sfc/test.h"

namespace sfc::geo {

SFC_TEST(vec_property) {
  auto v = Vector{};
  test::assert_eq(v.x(), 0.0);
  test::assert_eq(v.y(), 0.0);

  v = {1, 2};

  v = v.conj();
  test::assert_eq(v.x(), +1.0);
  test::assert_eq(v.y(), -2.0);

  auto p = v.as_point();
  test::assert_eq(v.x(), p.x());
  test::assert_eq(v.y(), p.y());
}

SFC_TEST(vec_direction) {
  const auto v0 = Vector{1, 0};
  test::assert_flt_eq(v0.to_angle().wrap_to_180(), 0);

  const auto v1 = Vector{1, 1};
  test::assert_flt_eq(v1.to_angle().wrap_to_180(), 45);

  const auto v2 = Vector{1, -1};
  test::assert_flt_eq(v2.to_angle().wrap_to_180(), -45);

  const auto v3 = Vector{-1, 1};
  test::assert_flt_eq(v3.to_angle().wrap_to_180(), 135);
}

SFC_TEST(vec_len) {
  const auto v0 = Vector{1, 0};
  const auto v1 = Vector{1, 1};

  test::assert_flt_eq(v0.length(), 1);
  test::assert_flt_eq(v0.length_sqr(), 1);

  test::assert_flt_eq(v1.length(), math::sqrt(2.0));
  test::assert_flt_eq(v1.length_sqr(), 2.0);
}

SFC_TEST(vec_unit) {
  const auto v0 = Vector{2, 0}.to_unit();
  const auto v1 = Vector{2, 2}.to_unit();

  test::assert_flt_eq(v0.x(), 1.0);
  test::assert_flt_eq(v0.y(), 0.0);

  test::assert_flt_eq(v1.x(), 1.0 / math::sqrt(2.0));
  test::assert_flt_eq(v1.y(), 1.0 / math::sqrt(2.0));
}

SFC_TEST(vec_norm) {
  const auto v0 = Vector{1, 0};
  const auto v1 = Vector{1, 1};

  test::assert_flt_eq(v0.norm1(), 1.0);
  test::assert_flt_eq(v1.norm1(), 2.0);

  test::assert_flt_eq(v0.norm2(), 1.0);
  test::assert_flt_eq(v1.norm2(), math::sqrt(2.0));
}

SFC_TEST(vec_prod) {
  const auto v0 = Vector{1, 0};
  const auto v1 = Vector{0, 1};

  const auto dot_prod = v0.dot_prod(v1);
  test::assert_flt_eq(dot_prod, 0);

  const auto cross_prod = v0.cross_prod(v1);
  test::assert_flt_eq(cross_prod, 1);
}

SFC_TEST(vec_ops) {
  const auto v0 = Vector{1, 0};
  const auto v1 = Vector{0, 1};

  const auto v2 = -v0;
  test::assert_flt_eq(v2.x(), -1.0);
  test::assert_flt_eq(v2.y(), +0.0);

  const auto v3 = v0 + v1;
  test::assert_flt_eq(v3.x(), 1.0);
  test::assert_flt_eq(v3.y(), 1.0);

  const auto v4 = v0 - v1;
  test::assert_flt_eq(v4.x(), +1.0);
  test::assert_flt_eq(v4.y(), -1.0);

  const auto v5 = 2 * v0;
  test::assert_flt_eq(v5.x(), 2.0);
  test::assert_flt_eq(v5.y(), 0.0);

  const auto v6 = v0 / 2;
  test::assert_flt_eq(v6.x(), 0.5);
  test::assert_flt_eq(v6.y(), 0.0);

  test::assert_flt_eq(v0.dot_prod(v0), 1.0);
  test::assert_flt_eq(v1.dot_prod(v1), 1.0);
  test::assert_flt_eq(v0.dot_prod(v1), 0.0);
}

}  // namespace sfc::geo
