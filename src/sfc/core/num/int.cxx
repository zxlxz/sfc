#include "sfc/test.h"

namespace sfc::num::test {

SFC_TEST(parse_i32) {
  panicking::assert_eq(Str{"0"}.parse<u32>(), Option<u32>{0});
  panicking::assert_eq(Str{"123"}.parse<i32>(), Option<i32>{123});
  panicking::assert_eq(Str{"-123"}.parse<i32>(), Option<i32>{-123});
  panicking::assert_eq(Str{"+123"}.parse<i32>(), Option<i32>{123});

  panicking::assert_false(Str{}.parse<i32>());
  panicking::assert_false(Str{"abc"}.parse<i32>());
  panicking::assert_false(Str{"123abc"}.parse<i32>());
  panicking::assert_false(Str{"123.456"}.parse<i32>());

  panicking::assert_false(Str{"++123"}.parse<i32>());
  panicking::assert_false(Str{"-+123"}.parse<i32>());
  panicking::assert_false(Str{"123+"}.parse<i32>());
  panicking::assert_false(Str{"123-"}.parse<i32>());
}

SFC_TEST(parse_u32) {
  panicking::assert_eq(Str{"0"}.parse<u32>(), Option<u32>{0});
  panicking::assert_eq(Str{"123"}.parse<u32>(), Option<u32>{123});
  panicking::assert_eq(Str{"+123"}.parse<u32>(), Option<u32>{123});

  panicking::assert_false(Str{}.parse<u32>());
  panicking::assert_false(Str{"abc"}.parse<u32>());
  panicking::assert_false(Str{"123abc"}.parse<u32>());
  panicking::assert_false(Str{"123.456"}.parse<u32>());

  panicking::assert_false(Str{"-123"}.parse<u32>());
  panicking::assert_false(Str{"123+"}.parse<u32>());
  panicking::assert_false(Str{"123.456"}.parse<u32>());
}

SFC_TEST(int2str_dec) {
  char buf[32];
  auto sbuf = Slice{buf};

  // char
  panicking::assert_eq(num::int2str(sbuf, 'a'), Str{"97"});
  panicking::assert_eq(num::int2str(sbuf, 'A'), Str{"65"});
  panicking::assert_eq(num::int2str(sbuf, '0'), Str{"48"});
  panicking::assert_eq(num::int2str(sbuf, u8('0')), Str{"48"});
  panicking::assert_eq(num::int2str(sbuf, i8('0')), Str{"48"});

  // short
  panicking::assert_eq(num::int2str(sbuf, u16(12345U)), Str{"12345"});
  panicking::assert_eq(num::int2str(sbuf, i16(12345)), Str{"12345"});
  panicking::assert_eq(num::int2str(sbuf, i16(-12345)), Str{"-12345"});

  // int
  panicking::assert_eq(num::int2str(sbuf, 123456), Str{"123456"});
  panicking::assert_eq(num::int2str(sbuf, -123456), Str{"-123456"});

  // long
  panicking::assert_eq(num::int2str(sbuf, 12345678L), Str{"12345678"});
  panicking::assert_eq(num::int2str(sbuf, -12345678L), Str{"-12345678"});

  // long long
  panicking::assert_eq(num::int2str(sbuf, 1234567890123456789LL), Str{"1234567890123456789"});
  panicking::assert_eq(num::int2str(sbuf, -1234567890123456789LL), Str{"-1234567890123456789"});
}

SFC_TEST(int2str_hex) {
  char buf[32];
  auto sbuf = Slice{buf};

  panicking::assert_eq(num::int2str(sbuf, 0), Str{"0"});

  // dec
  panicking::assert_eq(num::int2str(sbuf, 123U), Str{"123"});
  panicking::assert_eq(num::int2str(sbuf, 123, 'd'), Str{"123"});
  panicking::assert_eq(num::int2str(sbuf, -123, 'd'), Str{"-123"});

  // bin
  panicking::assert_eq(num::int2str(sbuf, 123U, 'b'), Str{"1111011"});
  panicking::assert_eq(num::int2str(sbuf, 123, 'b'), Str{"1111011"});
  panicking::assert_eq(num::int2str(sbuf, -123, 'b'), Str{"-1111011"});

  // oct
  panicking::assert_eq(num::int2str(sbuf, 123U, 'o'), Str{"173"});
  panicking::assert_eq(num::int2str(sbuf, 123, 'o'), Str{"173"});
  panicking::assert_eq(num::int2str(sbuf, -123, 'o'), Str{"-173"});

  // hex
  panicking::assert_eq(num::int2str(sbuf, 123U, 'x'), Str{"7b"});
  panicking::assert_eq(num::int2str(sbuf, 123, 'x'), Str{"7b"});
  panicking::assert_eq(num::int2str(sbuf, -123, 'x'), Str{"-7b"});

  panicking::assert_eq(num::int2str(sbuf, 123U, 'X'), Str{"7B"});
  panicking::assert_eq(num::int2str(sbuf, 123, 'X'), Str{"7B"});
  panicking::assert_eq(num::int2str(sbuf, -123, 'X'), Str{"-7B"});

  // ptr [width=12]
  const auto ptr = reinterpret_cast<const void*>(0x123abc);
  panicking::assert_eq(num::int2str(sbuf, ptr), Str{"000000123abc"});
  panicking::assert_eq(num::int2str(sbuf, ptr, 'x'), Str{"123abc"});
  panicking::assert_eq(num::int2str(sbuf, ptr, 'X'), Str{"123ABC"});
}

}  // namespace sfc::num::test
