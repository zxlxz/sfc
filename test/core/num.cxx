#include "sfc/test.h"

namespace sfc::num {

SFC_TEST(saturation_sub) {
  test::assert_eq(num::saturating_sub(1U, 2U), 0U);
  test::assert_eq(num::saturating_sub(2U, 1U), 1U);

  test::assert_eq(num::saturating_sub(1, 2), -1);
  test::assert_eq(num::saturating_sub(2, 1), +1);
}

SFC_TEST(parse_uint) {
  // normal
  test::assert_eq(Str{"123"}.parse<u8>(), Option<u8>{123});
  test::assert_eq(Str{"123"}.parse<u16>(), Option<u16>{123});
  test::assert_eq(Str{"123"}.parse<u32>(), Option<u32>{123});
  test::assert_eq(Str{"123"}.parse<u64>(), Option<u64>{123});

  // sign
  test::assert_true(Str{"+123"}.parse<u16>());
  test::assert_false(Str{"-123"}.parse<u16>());

  // range
  test::assert_true(Str{"255"}.parse<u8>());
  test::assert_false(Str{"256"}.parse<u8>());

  // bin
  test::assert_eq(Str{"0010"}.parse<u32>(), Option<u32>{8});
  test::assert_eq(Str{"0b0010"}.parse<u32>(), Option<u32>{2});
  test::assert_eq(Str{"0x0010"}.parse<u32>(), Option<u32>{16});
}

SFC_TEST(parse_sint) {
  // normal
  test::assert_eq(Str{"123"}.parse<i8>(), Option<i8>{123});
  test::assert_eq(Str{"123"}.parse<i16>(), Option<i16>{123});
  test::assert_eq(Str{"123"}.parse<i32>(), Option<i32>{123});
  test::assert_eq(Str{"123"}.parse<i64>(), Option<i64>{123});

  // sign
  test::assert_true(Str{"+123"}.parse<i16>());
  test::assert_true(Str{"-123"}.parse<i16>());

  // range
  test::assert_true(Str{"+127"}.parse<i8>());
  test::assert_true(Str{"-127"}.parse<i8>());
  test::assert_false(Str{"+128"}.parse<i8>());
  test::assert_false(Str{"-128"}.parse<i8>());

  // bin
  test::assert_eq(Str{"-0010"}.parse<i32>(), Option<i32>{-8});
  test::assert_eq(Str{"-0b0010"}.parse<i32>(), Option<i32>{-2});
  test::assert_eq(Str{"-0x0010"}.parse<i32>(), Option<i32>{-16});
}
}  // namespace sfc::num
