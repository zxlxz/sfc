#include "sfc/test/test.h"

namespace sfc::num::test {

SFC_TEST(flt_eq) {
  sfc::expect_flt_eq(1.0, 1.0);
  sfc::expect_flt_eq(-1.0, -1.0);
  sfc::expect_flt_ne(1.0, -1.0);

  sfc::expect_flt_eq(1.0, 1.0 + 1e-20);
  sfc::expect_flt_ne(1.0, 1.0 + 1e-10);

  sfc::expect_flt_eq(0.0, 0.0);
  sfc::expect_flt_eq(-0.0, +0.0);
  sfc::expect_flt_eq(-0.0, -0.0);

  sfc::expect_flt_ne(__builtin_nan(""), __builtin_nan(""));
  sfc::expect_flt_ne(__builtin_inf(), __builtin_inf());
  sfc::expect_flt_ne(-__builtin_inf(), -__builtin_inf());
  sfc::expect_flt_ne(__builtin_inf(), -__builtin_inf());
}

SFC_TEST(flt_to_str) {
  char buf[32] = {};

  // zero
  sfc::expect_eq(Flt{0.0}.to_str(buf, 0), "0");

  // positive
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 0), "123");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 1), "123.5");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 2), "123.46");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 3), "123.456");

  // negative
  sfc::expect_eq(Flt{-123.456}.to_str(buf, 0), "-123");
  sfc::expect_eq(Flt{-123.456}.to_str(buf, 1), "-123.5");
  sfc::expect_eq(Flt{-123.456}.to_str(buf, 2), "-123.46");
  sfc::expect_eq(Flt{-123.456}.to_str(buf, 3), "-123.456");

  // nan/inf
  sfc::expect_eq(Flt{+__builtin_nan("")}.to_str(buf), "nan");
  sfc::expect_eq(Flt{-__builtin_nan("")}.to_str(buf), "nan");
  sfc::expect_eq(Flt{+__builtin_inf()}.to_str(buf), "inf");
  sfc::expect_eq(Flt{-__builtin_inf()}.to_str(buf), "-inf");
}

SFC_TEST(flt_to_exp) {
  char buf[32] = {};

  // zero
  sfc::expect_eq(Flt{0.0}.to_str(buf, 0, 'e'), "0e+00");
  sfc::expect_eq(Flt{0.0}.to_str(buf, 0, 'E'), "0E+00");

  // exp
  sfc::expect_eq(Flt{1.234567e1}.to_str(buf, 3, 'e'), "1.235e+01");
  sfc::expect_eq(Flt{1.234567e2}.to_str(buf, 3, 'E'), "1.235E+02");
  sfc::expect_eq(Flt{-1.234567e-1}.to_str(buf, 3, 'e'), "-1.235e-01");
  sfc::expect_eq(Flt{-1.234567e-2}.to_str(buf, 3, 'E'), "-1.235E-02");

  // precision
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 2, 'e'), "1.23e+02");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 3, 'e'), "1.235e+02");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 4, 'e'), "1.2346e+02");
  sfc::expect_eq(Flt{+123.456}.to_str(buf, 5, 'e'), "1.23456e+02");

  // nan/inf
  sfc::expect_eq(Flt{+__builtin_nan("")}.to_str(buf, 2, 'e'), "nan");
  sfc::expect_eq(Flt{-__builtin_nan("")}.to_str(buf, 2, 'e'), "nan");
  sfc::expect_eq(Flt{+__builtin_inf()}.to_str(buf, 2, 'e'), "inf");
  sfc::expect_eq(Flt{-__builtin_inf()}.to_str(buf, 2, 'e'), "-inf");
}

SFC_TEST(flt_from_str) {
  // zero
  sfc::expect_eq(Flt<f64>::from_str("0.0"), Option{0.0});

  // valid
  sfc::expect_flt_eq(Flt<f64>::from_str("123.456").unwrap_or(0), 123.456);
  sfc::expect_flt_eq(Flt<f64>::from_str("-123.456").unwrap_or(0), -123.456);

  // invalid
  sfc::expect_false(Flt<f64>::from_str("abc"));
  sfc::expect_false(Flt<f64>::from_str("++12"));
  sfc::expect_false(Flt<f64>::from_str("+-12"));
  sfc::expect_false(Flt<f64>::from_str("12.34.56"));
  sfc::expect_false(Flt<f64>::from_str("12..3"));
  sfc::expect_false(Flt<f64>::from_str("12..3"));
}

SFC_TEST(flt_from_exp) {
  // valid
  sfc::expect_flt_eq(Flt<f64>::from_str("1.23456e2").unwrap_or(0), 123.456);
  sfc::expect_flt_eq(Flt<f64>::from_str("-1.23456E2").unwrap_or(0), -123.456);
  sfc::expect_flt_eq(Flt<f64>::from_str("1.23456e-2").unwrap_or(0), 0.0123456);
  sfc::expect_flt_eq(Flt<f64>::from_str("-1.23456E-2").unwrap_or(0), -0.0123456);

  // invalid
  sfc::expect_false(Flt<f64>::from_str("1.23e"));
  sfc::expect_false(Flt<f64>::from_str("1.23e++2"));
  sfc::expect_false(Flt<f64>::from_str("1.23e--2"));
}

}  // namespace sfc::num::test
