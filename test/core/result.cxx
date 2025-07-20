#include "sfc/test.h"

namespace sfc::result {

SFC_TEST(copy) {
  auto a = Result<f32, int>{1.0f};
  auto b = Result<f32, int>{1};

  panicking::assert_true(a.is_ok());
  panicking::assert_false(a.is_err());
  panicking::assert_eq(a.unwrap(), 1.0f);

  panicking::assert_false(b.is_ok());
  panicking::assert_true(b.is_err());
  panicking::assert_eq(b.unwrap_err(), 1);
}

SFC_TEST(move) {
  auto a = Result<String, int>{String::from("hello")};
  auto b = Result<String, int>{1};

  panicking::assert_true(a.is_ok());
  panicking::assert_false(a.is_err());
  panicking::assert_eq(mem::move(a).unwrap(), "hello");

  panicking::assert_false(b.is_ok());
  panicking::assert_true(b.is_err());
  panicking::assert_eq(mem::move(b).unwrap_err(), 1);
}
}  // namespace sfc::result
