#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(max_value) {
  static_assert(num::max_value<u8>() == __UINT8_MAX__);
  static_assert(num::max_value<u16>() == __UINT16_MAX__);
  static_assert(num::max_value<u32>() == __UINT32_MAX__);
  static_assert(num::max_value<u64>() == __UINT64_MAX__);

  static_assert(num::max_value<i8>() == __INT8_MAX__);
  static_assert(num::max_value<i16>() == __INT16_MAX__);
  static_assert(num::max_value<i32>() == __INT32_MAX__);
  static_assert(num::max_value<i64>() == __INT64_MAX__);
}

SFC_TEST(min_value) {
  static_assert(num::min_value<u8>() == 0U);
  static_assert(num::min_value<u16>() == 0U);
  static_assert(num::min_value<u32>() == 0U);
  static_assert(num::min_value<u64>() == 0U);

  static_assert(num::min_value<i8>() == -__INT8_MAX__ - 1);
  static_assert(num::min_value<i16>() == -__INT16_MAX__ - 1);
  static_assert(num::min_value<i32>() == -__INT32_MAX__ - 1);
  static_assert(num::min_value<i64>() == -__INT64_MAX__ - 1);
}

SFC_TEST(satruating_sub) {
  static_assert(num::saturating_sub(1U, 2U) == 0);
  static_assert(num::saturating_sub(2U, 1U) == 1);
}

SFC_TEST(next_power_of_two) {
  static_assert(num::next_power_of_two(1) == 1);
  static_assert(num::next_power_of_two(2) == 2);
  static_assert(num::next_power_of_two(3) == 4);
}

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
