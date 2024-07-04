#include "sfc/geo.h"
#include "sfc/test.h"

namespace sfc::geo {

SFC_TEST(angle_from_rad) {
  auto a1 = Angle::from_rad(1);
  test::assert_flt_eq(a1.as_deg(), 180.0 / PI);
  test::assert_flt_eq(a1.as_rad(), 1);
}

SFC_TEST(angle_from_deg) {
  auto a1 = Angle::from_deg(90);
  test::assert_flt_eq(a1.as_deg(), 90);
  test::assert_flt_eq(a1.as_rad(), PI / 2);
}

SFC_TEST(angle_wrap_to_pi) {
  test::assert_flt_eq(Angle{0}.wrap_to_pi(), 0);

  test::assert_flt_eq(Angle{+1}.wrap_to_pi(), +1);
  test::assert_flt_eq(Angle{-1}.wrap_to_pi(), -1);

  test::assert_flt_eq(Angle{1 + PI}.wrap_to_pi(), 1 - PI);
  test::assert_flt_eq(Angle{1 - PI}.wrap_to_pi(), 1 - PI);
}

SFC_TEST(angle_wrap_to_2pi) {
  test::assert_flt_eq(Angle{0}.wrap_to_2pi(), 0);

  test::assert_flt_eq(Angle{+1}.wrap_to_2pi(), +1);
  test::assert_flt_eq(Angle{-1}.wrap_to_2pi(), 2 * PI - 1);

  test::assert_flt_eq(Angle{1 + PI}.wrap_to_2pi(), 1 + PI);
  test::assert_flt_eq(Angle{1 - PI}.wrap_to_2pi(), 1 + PI);
}

SFC_TEST(angle_wrap_to_180) {
  test::assert_flt_eq(Angle::from_deg(0).wrap_to_180(), 0);

  test::assert_flt_eq(Angle::from_deg(+100).wrap_to_180(), +100);
  test::assert_flt_eq(Angle::from_deg(-100).wrap_to_180(), -100);

  test::assert_flt_eq(Angle::from_deg(+460).wrap_to_180(), +100);
  test::assert_flt_eq(Angle::from_deg(-460).wrap_to_180(), -100);
}

SFC_TEST(angle_wrap_to_360) {
  test::assert_flt_eq(Angle::from_deg(0).wrap_to_360(), 0);

  test::assert_flt_eq(Angle::from_deg(+100).wrap_to_360(), +100);
  test::assert_flt_eq(Angle::from_deg(-100).wrap_to_360(), +260);

  test::assert_flt_eq(Angle::from_deg(+460).wrap_to_360(), +100);
  test::assert_flt_eq(Angle::from_deg(-460).wrap_to_360(), +260);
}

SFC_TEST(angle_ops) {
  const auto a = Angle{1};
  const auto b = Angle{2};

  test::assert_flt_eq((a + b).value(), +3);
  test::assert_flt_eq((a - b).value(), -1);
  test::assert_flt_eq((2 * a).value(), +2);
  test::assert_flt_eq((b / 2).value(), +1);
}

}  // namespace sfc::geo
