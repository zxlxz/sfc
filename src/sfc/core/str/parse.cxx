#include "sfc/test.h"

namespace sfc::str::test {

SFC_TEST(parse_uint) {
  sfc::expect_eq(Str{"123"}.parse<u64>(), Option<u64>{123U});
  sfc::expect_eq(Str{"0"}.parse<u64>(), Option<u64>{0U});
  sfc::expect_eq(Str{"4294967295"}.parse<u32>(), Option<u32>{0xFFFFFFFFU});
  sfc::expect_eq(Str{"0xff"}.parse<u64>(), Option<u64>{0xFFU});
  sfc::expect_eq(Str{"0xFF"}.parse<u64>(), Option<u64>{0xFFU});
  sfc::expect_eq(Str{"0b1010"}.parse<u64>(), Option<u64>{0xAU});
  sfc::expect_eq(Str{"0755"}.parse<u64>(), Option<u64>{0755U});

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<u64>(), Option<u64>{});
  sfc::expect_eq(Str{"-123"}.parse<u64>(), Option<u64>{});
  sfc::expect_eq(Str{"abc"}.parse<u64>(), Option<u64>{});
}

SFC_TEST(parse_sint) {
  sfc::expect_eq(Str{"123"}.parse<i64>(), Option<i64>{123L});
  sfc::expect_eq(Str{"-123"}.parse<i64>(), Option<i64>{-123L});
  sfc::expect_eq(Str{"0"}.parse<i64>(), Option<i64>{0L});
  sfc::expect_eq(Str{"-2147483648"}.parse<i32>(), Option<i32>{-2147483648});
  sfc::expect_eq(Str{"2147483647"}.parse<i32>(), Option<i32>{2147483647});
  sfc::expect_eq(Str{"0xff"}.parse<i64>(), Option<i64>{0xFFL});
  sfc::expect_eq(Str{"-0xff"}.parse<i64>(), Option<i64>{-0xFFL});

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<i64>(), Option<i64>{});
  sfc::expect_eq(Str{"abc"}.parse<i64>(), Option<i64>{});
}

SFC_TEST(parse_flt) {
  sfc::expect_flt_eq(Str{"3.14"}.parse<f64>().unwrap(), 3.14);
  sfc::expect_flt_eq(Str{"-3.14"}.parse<f64>().unwrap(), -3.14);
  sfc::expect_flt_eq(Str{"0.0"}.parse<f64>().unwrap(), 0.0);
  sfc::expect_flt_eq(Str{"123.456"}.parse<f64>().unwrap(), 123.456);
  sfc::expect_flt_eq(Str{"1.5"}.parse<f32>().unwrap(), 1.5F);

  // Invalid cases
  sfc::expect_eq(Str{""}.parse<f64>(), Option<f64>{});
  sfc::expect_eq(Str{"abc"}.parse<f64>(), Option<f64>{});
}

SFC_TEST(parse_exp) {
  sfc::expect_flt_eq(Str{"1e10"}.parse<f64>().unwrap(), 1e10);
  sfc::expect_flt_eq(Str{"1E10"}.parse<f64>().unwrap(), 1E10);
  sfc::expect_flt_eq(Str{"1.5e2"}.parse<f64>().unwrap(), 150.0);
  sfc::expect_flt_eq(Str{"1.5e-2"}.parse<f64>().unwrap(), 0.015);
  sfc::expect_flt_eq(Str{"-1.5e2"}.parse<f64>().unwrap(), -150.0);
  sfc::expect_flt_eq(Str{"-1.5e-2"}.parse<f64>().unwrap(), -0.015);
  sfc::expect_flt_eq(Str{"1e-10"}.parse<f64>().unwrap(), 1e-10);
}

}  // namespace sfc::str::test
