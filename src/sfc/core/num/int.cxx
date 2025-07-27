#include "sfc/test.h"

namespace sfc::num::test {

SFC_TEST(parse_int) {
  panicking::assert_eq(Str{"0"}.parse<u32>(), Option<u32>{0});
  panicking::assert_eq(Str{"123"}.parse<int>(), Option<int>{123});
  panicking::assert_eq(Str{"-123"}.parse<int>(), Option<int>{-123});
  panicking::assert_eq(Str{"+123"}.parse<int>(), Option<int>{123});

  panicking::assert_false(Str{}.parse<int>());
  panicking::assert_false(Str{"abc"}.parse<int>());
  panicking::assert_false(Str{"123abc"}.parse<int>());
  panicking::assert_false(Str{"123.456"}.parse<int>());

  panicking::assert_false(Str{"++123"}.parse<int>());
  panicking::assert_false(Str{"-+123"}.parse<int>());
  panicking::assert_false(Str{"123+"}.parse<int>());
  panicking::assert_false(Str{"123-"}.parse<int>());
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

}  // namespace sfc::num