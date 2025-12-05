#include "sfc/test.h"

namespace sfc::num::test {

SFC_TEST(fmt_fix) {
  panicking::expect_eq(string::format("{.0f}", +123.456), "123");
  panicking::expect_eq(string::format("{.0f}", -123.456), "-123");

  panicking::expect_eq(string::format("{.2f}", +123.456), "123.46");
  panicking::expect_eq(string::format("{.2f}", -123.456), "-123.46");

  panicking::expect_eq(string::format("{+.2f}", +123.456), "+123.46");
  panicking::expect_eq(string::format("{-.2f}", -123.456), "-123.46");

  panicking::expect_eq(string::format("{+7.2f}", +123.456), "+123.46");
  panicking::expect_eq(string::format("{+7.2f}", -123.456), "-123.46");

  panicking::expect_eq(string::format("{-7.2f}", +123.456), " 123.46");
  panicking::expect_eq(string::format("{-7.2f}", -123.456), "-123.46");
}

SFC_TEST(fmt_exp) {
  panicking::expect_eq(string::format("{.2e}", +123.456), "1.23e+02");
  panicking::expect_eq(string::format("{.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{+.2e}", +123.456), "+1.23e+02");
  panicking::expect_eq(string::format("{-.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{+8.2e}", +123.456), "+1.23e+02");
  panicking::expect_eq(string::format("{+8.2e}", -123.456), "-1.23e+02");

  panicking::expect_eq(string::format("{-8.2e}", +123.456), " 1.23e+02");
  panicking::expect_eq(string::format("{-8.2e}", -123.456), "-1.23e+02");
}

SFC_TEST(parse_f32) {
  panicking::expect_flt_eq(Str{"0"}.parse<f32>().unwrap(), 0);
  panicking::expect_flt_eq(Str{"12"}.parse<f32>().unwrap(), 12);
  panicking::expect_flt_eq(Str{"12.34"}.parse<f32>().unwrap(), 12.34f);
  panicking::expect_flt_eq(Str{"-12.34"}.parse<f32>().unwrap(), -12.34f);
  panicking::expect_flt_eq(Str{"+12.34"}.parse<f32>().unwrap(), 12.34f);

  panicking::expect(!Str{"abc"}.parse<f32>());
  panicking::expect(!Str{"12ab"}.parse<f32>());
  panicking::expect(!Str{"ab12"}.parse<f32>());
  panicking::expect(!Str{"12.34.56"}.parse<f32>());
}

SFC_TEST(parse_f64) {
  panicking::expect_flt_eq(Str{"0"}.parse<f64>().unwrap(), 0);
  panicking::expect_flt_eq(Str{"12"}.parse<f64>().unwrap(), 12);
  panicking::expect_flt_eq(Str{"12.34"}.parse<f64>().unwrap(), 12.34);
  panicking::expect_flt_eq(Str{"-12.34"}.parse<f64>().unwrap(), -12.34);
  panicking::expect_flt_eq(Str{"+12.34"}.parse<f64>().unwrap(), 12.34);

  panicking::expect(!Str{"abc"}.parse<f64>());
  panicking::expect(!Str{"12ab"}.parse<f64>());
  panicking::expect(!Str{"ab12"}.parse<f64>());
  panicking::expect(!Str{"12.34.56"}.parse<f64>());
}

}  // namespace sfc::num::test
