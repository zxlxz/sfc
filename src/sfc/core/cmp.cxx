#include "sfc/test.h"
#include "sfc/core/cmp.h"

namespace sfc::cmp::test {

SFC_TEST(max) {
  static_assert(trait::same_<decltype(cmp::max(i32{1}, 2)), i32>);
  static_assert(trait::same_<decltype(cmp::max(u32{1}, 2)), u32>);
  static_assert(trait::same_<decltype(cmp::max(i64{1}, 2)), i64>);

  static_assert(cmp::max(0, -1) == 0);
  static_assert(cmp::max(1, 2) == 2);
}

SFC_TEST(min) {
  static_assert(trait::same_<decltype(cmp::min(i32{1}, 2)), i32>);
  static_assert(trait::same_<decltype(cmp::min(u32{1}, 2)), u32>);
  static_assert(trait::same_<decltype(cmp::min(i64{1}, 2)), i64>);

  static_assert(cmp::min(0, -1) == -1);
  static_assert(cmp::min(1, 2) == 1);
}

}  // namespace sfc::cmp::test
