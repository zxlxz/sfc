#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(flt_eq) {
  sfc::expect_flt_eq(1.0, 1.0);
  sfc::expect_flt_eq(-1.0, -1.0);
  sfc::expect_flt_ne(1.0, -1.0);

  sfc::expect_flt_eq(1.0, 1.0 + 1e-20);
  sfc::expect_flt_ne(1.0, 1.0 + 1e-10);

  sfc::expect_flt_eq(0.0, 0.0);
  sfc::expect_flt_eq(-0.0, +0.0);
  sfc::expect_flt_eq(-0.0, -0.0);

  sfc::expect_flt_ne(__builtin_nan(""), __builtin_nan(""));
  sfc::expect_flt_ne(__builtin_inf(), __builtin_inf());
  sfc::expect_flt_ne(-__builtin_inf(), -__builtin_inf());
  sfc::expect_flt_ne(__builtin_inf(), -__builtin_inf());
}

}  // namespace sfc::num::test
