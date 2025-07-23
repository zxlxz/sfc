#include "sfc/test.h"

namespace sfc::result {

SFC_TEST(copy) {
  auto a = Result<f32, int>{1.0f};
  auto b = Result<f32, int>{1};

  panicking::assert_true(!!a);
  panicking::assert_eq(*a, 1.0f);

  panicking::assert_false(!!b);
  panicking::assert_eq(~b, 1);
}

SFC_TEST(move) {
  auto a = Result<String, int>{String::from("hello")};
  auto b = Result<String, int>{1};

  panicking::assert_true(!!a);
  panicking::assert_eq(*a, "hello");

  panicking::assert_false(!!b);
  panicking::assert_eq(~b, 1);
}
}  // namespace sfc::result
