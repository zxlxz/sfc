#include "sfc/test.h"

namespace sfc::string::test {

SFC_TEST(property) {
  const auto test_str = String::from("abc");
  const auto null_str = String::from("");

  panicking::expect_eq(test_str.len(), 3U);
  panicking::expect(!test_str.is_empty());

  panicking::expect_eq(null_str.len(), 0U);
  panicking::expect(null_str.is_empty());
}

SFC_TEST(index) {
  const auto s = String::from("abc");

  panicking::expect_eq(s[0], 'a');
  panicking::expect_eq(s[1], 'b');
  panicking::expect_eq(s[2], 'c');
}

SFC_TEST(index_range) {
  const auto s = String::from("abc");

  panicking::expect_eq((s[{0, $}]), "abc");
  panicking::expect_eq((s[{0, 2}]), "ab");
  panicking::expect_eq((s[{0, 8}]), "abc");

  panicking::expect_eq((s[{1, $}]), "bc");
  panicking::expect_eq((s[{1, 2}]), "b");

  panicking::expect_eq((s[{8, 9}]), "");
}

SFC_TEST(cmp) {
  const auto s = String::from("abcd");
  panicking::expect_eq(s, "abcd");
  panicking::expect_ne(s, "abc");
}

SFC_TEST(find) {
  const auto s = String::from("abc");

  panicking::expect_eq(s.find('a'), Option<usize>{0UL});
  panicking::expect_eq(s.find('b'), Option<usize>{1UL});
  panicking::expect_eq(s.rfind('c'), Option<usize>{2UL});
  panicking::expect_eq(s.find('d'), Option<usize>{});
}

SFC_TEST(rfind) {
  const auto s = String::from("abc");

  panicking::expect_eq(s.rfind('a'), Option<usize>{0UL});
  panicking::expect_eq(s.rfind('b'), Option<usize>{1UL});
  panicking::expect_eq(s.rfind('c'), Option<usize>{2UL});
  panicking::expect_eq(s.rfind('d'), Option<usize>{});
}

}  // namespace sfc::string::test
