#include "sfc/test/ut.h"

namespace sfc::num::test {

SFC_TEST(flt_eq) {
  panicking::expect(num::flt_eq_ulp(1.0, 1.0));
  panicking::expect(num::flt_eq_ulp(-1.0, -1.0));
  panicking::expect(!num::flt_eq_ulp(1.0, -1.0));

  panicking::expect(num::flt_eq_ulp(1.0, 1.0 + 1e-20));
  panicking::expect(!num::flt_eq_ulp(1.0, 1.0 + 1e-10));

  panicking::expect(num::flt_eq_ulp(0.0, 0.0));
  panicking::expect(num::flt_eq_ulp(-0.0, +0.0));
  panicking::expect(num::flt_eq_ulp(-0.0, -0.0));

  panicking::expect(!num::flt_eq_ulp(__builtin_nan(""), __builtin_nan("")));
  panicking::expect(!num::flt_eq_ulp(__builtin_inf(), __builtin_inf()));
  panicking::expect(!num::flt_eq_ulp(-__builtin_inf(), -__builtin_inf()));
  panicking::expect(!num::flt_eq_ulp(__builtin_inf(), -__builtin_inf()));
}

SFC_TEST(flt_to_str) {
  char buf[32] = {};

  // zero
  panicking::expect_eq(to_str(buf, 0.0, 0), "0");

  // positive
  panicking::expect_eq(to_str(buf, +123.456, 0), "123");
  panicking::expect_eq(to_str(buf, +123.456, 1), "123.5");
  panicking::expect_eq(to_str(buf, +123.456, 2), "123.46");
  panicking::expect_eq(to_str(buf, +123.456, 3), "123.456");

  // negative
  panicking::expect_eq(to_str(buf, -123.456, 0), "-123");
  panicking::expect_eq(to_str(buf, -123.456, 1), "-123.5");
  panicking::expect_eq(to_str(buf, -123.456, 2), "-123.46");
  panicking::expect_eq(to_str(buf, -123.456, 3), "-123.456");

  // nan/inf
  panicking::expect_eq(to_str(buf, +__builtin_nan("")), "nan");
  panicking::expect_eq(to_str(buf, -__builtin_nan("")), "nan");
  panicking::expect_eq(to_str(buf, +__builtin_inf()), "inf");
  panicking::expect_eq(to_str(buf, -__builtin_inf()), "-inf");
}

SFC_TEST(flt_to_exp) {
  char buf[32] = {};

  // zero
  panicking::expect_eq(to_str(buf, 0.0, 0, 'e'), "0e+00");
  panicking::expect_eq(to_str(buf, 0.0, 0, 'E'), "0E+00");

  // exp
  panicking::expect_eq(to_str(buf, 1.234567e1, 3, 'e'), "1.235e+01");
  panicking::expect_eq(to_str(buf, 1.234567e2, 3, 'E'), "1.235E+02");

  panicking::expect_eq(to_str(buf, -1.234567e-1, 3, 'e'), "-1.235e-01");
  panicking::expect_eq(to_str(buf, -1.234567e-2, 3, 'E'), "-1.235E-02");

  // precision
  panicking::expect_eq(to_str(buf, +123.456, 2, 'e'), "1.23e+02");
  panicking::expect_eq(to_str(buf, +123.456, 3, 'e'), "1.235e+02");
  panicking::expect_eq(to_str(buf, +123.456, 4, 'e'), "1.2346e+02");
  panicking::expect_eq(to_str(buf, +123.456, 5, 'e'), "1.23456e+02");

  // nan/inf
  panicking::expect_eq(to_str(buf, +__builtin_nan(""), 2, 'e'), "nan");
  panicking::expect_eq(to_str(buf, -__builtin_nan(""), 2, 'e'), "nan");
  panicking::expect_eq(to_str(buf, +__builtin_inf(), 2, 'e'), "inf");
  panicking::expect_eq(to_str(buf, -__builtin_inf(), 2, 'e'), "-inf");
}

SFC_TEST(flt_from_str) {
  // zero
  panicking::expect_eq(from_str<f64>("0.0"), Option{0.0});

  // valid
  panicking::expect_flt_eq(from_str<f64>("123.456").unwrap_or(0), 123.456);
  panicking::expect_flt_eq(from_str<f64>("-123.456").unwrap_or(0), -123.456);

  // invalid
  panicking::expect(!from_str<f64>("abc"));
  panicking::expect(!from_str<f64>("++12"));
  panicking::expect(!from_str<f64>("+-12"));
  panicking::expect(!from_str<f64>("12.34.56"));
  panicking::expect(!from_str<f64>("12..3"));
  panicking::expect(!from_str<f64>("12..3"));
}

SFC_TEST(flt_from_exp) {
  // valid
  panicking::expect_flt_eq(from_str<f64>("1.23456e2").unwrap_or(0), 123.456);
  panicking::expect_flt_eq(from_str<f64>("-1.23456E2").unwrap_or(0), -123.456);
  panicking::expect_flt_eq(from_str<f64>("1.23456e-2").unwrap_or(0), 0.0123456);
  panicking::expect_flt_eq(from_str<f64>("-1.23456E-2").unwrap_or(0), -0.0123456);

  // invalid
  panicking::expect(!from_str<f64>("1.23e"));
  panicking::expect(!from_str<f64>("1.23e++2"));
  panicking::expect(!from_str<f64>("1.23e--2"));
}

}  // namespace sfc::num::test
