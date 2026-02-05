#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(flt_eq) {
  panicking::expect_flt_eq(1.0, 1.0);
  panicking::expect_flt_eq(-1.0, -1.0);
  panicking::expect_flt_ne(1.0, -1.0);

  panicking::expect_flt_eq(1.0, 1.0 + 1e-20);
  panicking::expect_flt_ne(1.0, 1.0 + 1e-10);

  panicking::expect_flt_eq(0.0, 0.0);
  panicking::expect_flt_eq(-0.0, +0.0);
  panicking::expect_flt_eq(-0.0, -0.0);

  panicking::expect_flt_ne(__builtin_nan(""), __builtin_nan(""));
  panicking::expect_flt_ne(__builtin_inf(), __builtin_inf());
  panicking::expect_flt_ne(-__builtin_inf(), -__builtin_inf());
  panicking::expect_flt_ne(__builtin_inf(), -__builtin_inf());
}

SFC_TEST(flt_to_str) {
  char buf[32] = {};

  // zero
  panicking::expect_eq(Flt{0.0}.to_str(buf, 0), "0");

  // positive
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 0), "123");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 1), "123.5");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 2), "123.46");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 3), "123.456");

  // negative
  panicking::expect_eq(Flt{-123.456}.to_str(buf, 0), "-123");
  panicking::expect_eq(Flt{-123.456}.to_str(buf, 1), "-123.5");
  panicking::expect_eq(Flt{-123.456}.to_str(buf, 2), "-123.46");
  panicking::expect_eq(Flt{-123.456}.to_str(buf, 3), "-123.456");

  // nan/inf
  panicking::expect_eq(Flt{+__builtin_nan("")}.to_str(buf), "nan");
  panicking::expect_eq(Flt{-__builtin_nan("")}.to_str(buf), "nan");
  panicking::expect_eq(Flt{+__builtin_inf()}.to_str(buf), "inf");
  panicking::expect_eq(Flt{-__builtin_inf()}.to_str(buf), "-inf");
}

SFC_TEST(flt_to_exp) {
  char buf[32] = {};

  // zero
  panicking::expect_eq(Flt{0.0}.to_str(buf, 0, 'e'), "0e+00");
  panicking::expect_eq(Flt{0.0}.to_str(buf, 0, 'E'), "0E+00");

  // exp
  panicking::expect_eq(Flt{1.234567e1}.to_str(buf, 3, 'e'), "1.235e+01");
  panicking::expect_eq(Flt{1.234567e2}.to_str(buf, 3, 'E'), "1.235E+02");
  panicking::expect_eq(Flt{-1.234567e-1}.to_str(buf, 3, 'e'), "-1.235e-01");
  panicking::expect_eq(Flt{-1.234567e-2}.to_str(buf, 3, 'E'), "-1.235E-02");

  // precision
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 2, 'e'), "1.23e+02");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 3, 'e'), "1.235e+02");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 4, 'e'), "1.2346e+02");
  panicking::expect_eq(Flt{+123.456}.to_str(buf, 5, 'e'), "1.23456e+02");

  // nan/inf
  panicking::expect_eq(Flt{+__builtin_nan("")}.to_str(buf, 2, 'e'), "nan");
  panicking::expect_eq(Flt{-__builtin_nan("")}.to_str(buf, 2, 'e'), "nan");
  panicking::expect_eq(Flt{+__builtin_inf()}.to_str(buf, 2, 'e'), "inf");
  panicking::expect_eq(Flt{-__builtin_inf()}.to_str(buf, 2, 'e'), "-inf");
}

SFC_TEST(flt_from_str) {
  // zero
  panicking::expect_eq(Flt<f64>::from_str("0.0"), Option{0.0});

  // valid
  panicking::expect_flt_eq(Flt<f64>::from_str("123.456").unwrap_or(0), 123.456);
  panicking::expect_flt_eq(Flt<f64>::from_str("-123.456").unwrap_or(0), -123.456);

  // invalid
  panicking::expect_false(Flt<f64>::from_str("abc"));
  panicking::expect_false(Flt<f64>::from_str("++12"));
  panicking::expect_false(Flt<f64>::from_str("+-12"));
  panicking::expect_false(Flt<f64>::from_str("12.34.56"));
  panicking::expect_false(Flt<f64>::from_str("12..3"));
  panicking::expect_false(Flt<f64>::from_str("12..3"));
}

SFC_TEST(flt_from_exp) {
  // valid
  panicking::expect_flt_eq(Flt<f64>::from_str("1.23456e2").unwrap_or(0), 123.456);
  panicking::expect_flt_eq(Flt<f64>::from_str("-1.23456E2").unwrap_or(0), -123.456);
  panicking::expect_flt_eq(Flt<f64>::from_str("1.23456e-2").unwrap_or(0), 0.0123456);
  panicking::expect_flt_eq(Flt<f64>::from_str("-1.23456E-2").unwrap_or(0), -0.0123456);

  // invalid
  panicking::expect_false(Flt<f64>::from_str("1.23e"));
  panicking::expect_false(Flt<f64>::from_str("1.23e++2"));
  panicking::expect_false(Flt<f64>::from_str("1.23e--2"));
}

}  // namespace sfc::num::test
