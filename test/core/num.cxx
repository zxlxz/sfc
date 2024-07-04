#include "sfc/test.h"

namespace sfc::num {

SFC_TEST(sint) {
  test::assert_eq(num::max_value<i8>(), __INT8_MAX__);
  test::assert_eq(num::max_value<i16>(), __INT16_MAX__);
  test::assert_eq(num::max_value<i32>(), __INT32_MAX__);
  test::assert_eq(num::max_value<i64>(), __INT64_MAX__);

  test::assert_eq(num::min_value<i8>(), -__INT8_MAX__ - 1);
  test::assert_eq(num::min_value<i16>(), -__INT16_MAX__ - 1);
  test::assert_eq(num::min_value<i32>(), -__INT32_MAX__ - 1);
  test::assert_eq(num::min_value<i64>(), -__INT64_MAX__ - 1);
}

SFC_TEST(uint) {
  test::assert_eq(num::max_value<u8>(), __UINT8_MAX__);
  test::assert_eq(num::max_value<u16>(), __UINT16_MAX__);
  test::assert_eq(num::max_value<u32>(), __UINT32_MAX__);
  test::assert_eq(num::max_value<u64>(), __UINT64_MAX__);

  test::assert_eq(num::min_value<u8>(), 0U);
  test::assert_eq(num::min_value<u16>(), 0U);
  test::assert_eq(num::min_value<u32>(), 0U);
  test::assert_eq(num::min_value<u64>(), 0U);
}

SFC_TEST(saturation_sub) {
  test::assert_eq(num::saturating_sub(1U, 2U), 0U);
  test::assert_eq(num::saturating_sub(2U, 1U), 1U);

  test::assert_eq(num::saturating_sub(1, 2), -1);
  test::assert_eq(num::saturating_sub(2, 1), +1);
}

}  // namespace sfc::num
