#include "sfc/test.h"

namespace sfc::num {

SFC_TEST(saturation_sub) {
  panicking::assert_eq(num::saturating_sub(1U, 2U), 0U);
  panicking::assert_eq(num::saturating_sub(2U, 1U), 1U);

  panicking::assert_eq(num::saturating_sub(1, 2), -1);
  panicking::assert_eq(num::saturating_sub(2, 1), +1);
}

SFC_TEST(parse_uint) {
  // normal
  panicking::assert_eq(Str{"123"}.parse<u8>(), Option<u8>{123});
  panicking::assert_eq(Str{"123"}.parse<u16>(), Option<u16>{123});
  panicking::assert_eq(Str{"123"}.parse<u32>(), Option<u32>{123});
  panicking::assert_eq(Str{"123"}.parse<u64>(), Option<u64>{123});

  // sign
  panicking::assert_true(Str{"+123"}.parse<u16>());
  panicking::assert_false(Str{"-123"}.parse<u16>());

  // range
  panicking::assert_true(Str{"255"}.parse<u8>());
  panicking::assert_false(Str{"256"}.parse<u8>());

  // bin
  panicking::assert_eq(Str{"0010"}.parse<u32>(), Option<u32>{8});
  panicking::assert_eq(Str{"0b0010"}.parse<u32>(), Option<u32>{2});
  panicking::assert_eq(Str{"0x0010"}.parse<u32>(), Option<u32>{16});
}

SFC_TEST(parse_sint) {
  // normal
  panicking::assert_eq(Str{"123"}.parse<i8>(), Option<i8>{123});
  panicking::assert_eq(Str{"123"}.parse<i16>(), Option<i16>{123});
  panicking::assert_eq(Str{"123"}.parse<i32>(), Option<i32>{123});
  panicking::assert_eq(Str{"123"}.parse<i64>(), Option<i64>{123});

  // sign
  panicking::assert_true(Str{"+123"}.parse<i16>());
  panicking::assert_true(Str{"-123"}.parse<i16>());

  // range
  panicking::assert_true(Str{"+127"}.parse<i8>());
  panicking::assert_true(Str{"-127"}.parse<i8>());
  panicking::assert_false(Str{"+128"}.parse<i8>());
  panicking::assert_false(Str{"-128"}.parse<i8>());

  // bin
  panicking::assert_eq(Str{"-0010"}.parse<i32>(), Option<i32>{-8});
  panicking::assert_eq(Str{"-0b0010"}.parse<i32>(), Option<i32>{-2});
  panicking::assert_eq(Str{"-0x0010"}.parse<i32>(), Option<i32>{-16});
}
}  // namespace sfc::num
