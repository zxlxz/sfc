#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(max_value) {
  static_assert(num::max_value<u8>() == 0xFFU);
  static_assert(num::max_value<u16>() == 0xFFFFU);
  static_assert(num::max_value<u32>() == 0xFFFFFFFFU);
  static_assert(num::max_value<u64>() == 0xFFFFFFFFFFFFFFFFU);

  static_assert(num::max_value<i8>() == 0x7F);
  static_assert(num::max_value<i16>() == 0x7FFF);
  static_assert(num::max_value<i32>() == 0x7FFFFFFF);
  static_assert(num::max_value<i64>() == 0x7FFFFFFFFFFFFFFF);
}

SFC_TEST(min_value) {
  static_assert(num::min_value<i8>() == -0x80);
  static_assert(num::min_value<i16>() == -0x8000);
  static_assert(num::min_value<i32>() == -0x80000000);
  static_assert(num::min_value<i64>() == -0x8000000000000000);
}

SFC_TEST(saturating_sub) {
  static_assert(num::saturating_sub(1U, 2U) == 0);
  static_assert(num::saturating_sub(2U, 1U) == 1);
}

SFC_TEST(next_power_of_two) {
  static_assert(num::next_power_of_two(1U) == 1);
  static_assert(num::next_power_of_two(2U) == 2);
  static_assert(num::next_power_of_two(3U) == 4);
  static_assert(num::next_power_of_two(0x80000000U) == 0x80000000U);
  static_assert(num::next_power_of_two(0x7FFFFFFFFU) == 0x800000000);
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
}

}  // namespace sfc::num::test
