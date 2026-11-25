#include "sfc/core/variant.h"

#include "sfc/test/test.h"

namespace sfc::variant::test {

SFC_TEST(variant_basic) {
  const auto a = Variant<i32, f32>{2};
  sfc::assert_eq(a.is<i32>(), true);
  sfc::assert_eq(a.as<i32>(), Option{2});
  sfc::assert_eq(a.as<f32>(), Option<f32>{});

  const auto b = Variant<i32, f32>{3.0f};
  sfc::assert_eq(b.is<f32>(), true);
  sfc::assert_eq(b.as<i32>(), Option<i32>{});
  sfc::assert_eq(b.as<f32>(), Option{3.0f});
}

SFC_TEST(variant_move) {
  auto a = Variant<i32, f32>{2};
  auto b = mem::move(a);
  sfc::assert_eq(b.as<i32>(), Option{2});
}

SFC_TEST(variant_is) {
  auto a = Variant<i32, f32>{2};
  sfc::assert_eq(a.is<i32>(), true);
  sfc::assert_eq(a.is<f32>(), false);
}

SFC_TEST(variant_as) {
  const auto a = Variant<bool, i32>{true};
  sfc::assert_eq(a.as<bool>(), Option{true});
  sfc::assert_eq(a.as<i32>(), None{});
}

SFC_TEST(variant_as_mut) {
  auto a = Variant<bool, u32>{1U};
  sfc::assert_eq(a.as_mut<u32>(), Option{1U});
  sfc::assert_eq(a.as_mut<bool>(), None{});

  a.as_mut<u32>().map([](auto& val) {
    val += 1;
    return Unit{};
  });

  sfc::assert_eq(a.as_mut<u32>(), Option{2U});
}

SFC_TEST(variant_map) {
  auto a = Variant<i32, f32>{2};
  a.map([](const auto& val) { sfc::assert_eq(val, 2); });
  a.map_mut([](auto& val) { val += 1; });
  a.map([](const auto& val) { sfc::assert_eq(val, 3); });
}

// Variant::fmt
SFC_TEST(variant_fmt) {
  auto a = Variant<i32, f32>{2};
  sfc::assert_eq(string::format("{}", a), "2");

  auto b = Variant<i32, bool>{false};
  sfc::assert_eq(string::format("{}", b), "false");
}

}  // namespace sfc::variant::test
