#include "sfc/core/variant.h"

#include "sfc/test/test.h"

namespace sfc::variant::test {

SFC_TEST(variant_basic) {
  auto a = Variant<i32, f32>{2};
  panicking::expect_true(a.is<i32>());
  panicking::expect_eq(a.as<i32>(), Option<i32>{2});
  panicking::expect_eq(a.as<f32>(), Option<f32>{});

  auto b = Variant<i32, f32>{3.0f};
  panicking::expect_true(b.is<f32>());
  panicking::expect_eq(b.as<i32>(), Option<i32>{});
  panicking::expect_eq(b.as<f32>(), Option<f32>{3.0f});
}

// Variant::map, Variant::map_mut
SFC_TEST(map) {
  auto a = Variant<i32, f32>{2};
  a.map([](auto x) { panicking::expect_eq(x, 2); });

  a.map_mut([](auto& x) { x += 3; });
  panicking::expect_eq(a.as<i32>(), Option{i32{5}});

  auto c = Variant<i32, f32>{3.0f};
  c.map([](auto x) { panicking::expect_eq(x, 3.0f); });
}

// Variant::fmt
SFC_TEST(fmt) {
  auto a = Variant<i32, f32>{2};
  panicking::expect_eq(string::format("{}", a), "2");

  auto b = Variant<i32, f32>{3.5f};
  panicking::expect_eq(string::format("{}", b), "3.5000");
}

}  // namespace sfc::variant::test
