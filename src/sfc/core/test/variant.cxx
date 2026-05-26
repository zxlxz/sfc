#include "sfc/core/variant.h"

#include "sfc/test/test.h"

namespace sfc::variant::test {

SFC_TEST(variant_basic) {
  const auto a = Variant<i32, f32>{2};
  sfc::expect_true(a.is<i32>());
  sfc::expect_eq(a.as<i32>(), Option{2});
  sfc::expect_eq(a.as<f32>(), Option{});

  const auto b = Variant<i32, f32>{3.0f};
  sfc::expect_true(b.is<f32>());
  sfc::expect_eq(b.as<i32>(), Option{});
  sfc::expect_eq(b.as<f32>(), Option{3.0f});
}

SFC_TEST(variant_copy) {
  const auto a = Variant<i32, f32>{2};
  sfc::expect_eq(a.as<i32>(), Option{2});
  sfc::expect_eq(auto{a}.as<i32>(), Option{2});
}

SFC_TEST(variant_move) {
  auto a = Variant<i32, f32>{2};
  auto b = mem::move(a);
  sfc::expect_eq(b.as<i32>(), Option{2});
}

// Variant::fmt
SFC_TEST(variant_fmt) {
  auto a = Variant<i32, f32>{2};
  sfc::expect_eq(string::format("{}", a), "2");

  auto b = Variant<i32, bool>{false};
  sfc::expect_eq(string::format("{}", b), "false");
}

}  // namespace sfc::variant::test
