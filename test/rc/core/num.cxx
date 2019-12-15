#include "rc/test.h"

#include "rc/core/num.h"
#include "rc/core/str.h"

namespace rc::num {

rc_test(trait) {
  static_assert(is_signed<i8>());
  static_assert(is_signed<i16>());
  static_assert(is_signed<i32>());
  static_assert(is_signed<i64>());

  static_assert(is_unsigned<u8>());
  static_assert(is_unsigned<u16>());
  static_assert(is_unsigned<u32>());
  static_assert(is_unsigned<u64>());

  static_assert(is_floating_point<f32>());
  static_assert(is_floating_point<f64>());
}

rc_test(int_from_str) {
  assert_eq(Str{"1"}.parse<u32>(), Option{1u});
  assert_eq(Str{"11"}.parse<u32>(), Option{11u});
  assert_eq(Str{"128"}.parse<i32>(), Option{128});

  assert_eq(Str{"-15"}.parse<i32>(), Option{-15});
}

}  // namespace rc::num
