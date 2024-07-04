#include "sfc/test.h"

namespace sfc::num::test {

SFC_TEST(parse_f32) {
  panicking::assert_flt_eq(Str{"0"}.parse<f32>().unwrap(), 0);
  panicking::assert_flt_eq(Str{"12"}.parse<f32>().unwrap(), 12);
  panicking::assert_flt_eq(Str{"12.34"}.parse<f32>().unwrap(), 12.34f);
  panicking::assert_flt_eq(Str{"-12.34"}.parse<f32>().unwrap(), -12.34f);
  panicking::assert_flt_eq(Str{"+12.34"}.parse<f32>().unwrap(), 12.34f);

  panicking::assert_false(Str{"abc"}.parse<f32>());
  panicking::assert_false(Str{"12ab"}.parse<f32>());
  panicking::assert_false(Str{"ab12"}.parse<f32>());
  panicking::assert_false(Str{"12.34.56"}.parse<f32>());
}

SFC_TEST(parse_f64) {
  panicking::assert_flt_eq(Str{"0"}.parse<f64>().unwrap(), 0);
  panicking::assert_flt_eq(Str{"12"}.parse<f64>().unwrap(), 12);
  panicking::assert_flt_eq(Str{"12.34"}.parse<f64>().unwrap(), 12.34);
  panicking::assert_flt_eq(Str{"-12.34"}.parse<f64>().unwrap(), -12.34);
  panicking::assert_flt_eq(Str{"+12.34"}.parse<f64>().unwrap(), 12.34);

  panicking::assert_false(Str{"abc"}.parse<f64>());
  panicking::assert_false(Str{"12ab"}.parse<f64>());
  panicking::assert_false(Str{"ab12"}.parse<f64>());
  panicking::assert_false(Str{"12.34.56"}.parse<f64>());
}

SFC_TEST(flt2str_dec) {
  char buf[32];
  auto sbuf = Slice{buf};

  // f32
  panicking::assert_eq(num::flt2str(sbuf, 0.0f), Str{"0.000000"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34f), Str{"12.340000"});
  panicking::assert_eq(num::flt2str(sbuf, -12.34f), Str{"-12.340000"});

  // f64
  panicking::assert_eq(num::flt2str(sbuf, 0.0), Str{"0.000000"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34), Str{"12.340000"});
  panicking::assert_eq(num::flt2str(sbuf, -12.34), Str{"-12.340000"});
  panicking::assert_eq(num::flt2str(sbuf, 12345678.9), Str{"12345678.900000"});

  // width
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 0), Str{"12"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 2), Str{"12.34"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 4), Str{"12.3400"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 6), Str{"12.340000"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 8), Str{"12.34000000"});

  // exp
  panicking::assert_eq(num::flt2str(sbuf, 0.f, 4, 'e'), Str{"0.0000e+00"});
  panicking::assert_eq(num::flt2str(sbuf, 12.34, 4, 'e'), Str{"1.2340e+01"});
  panicking::assert_eq(num::flt2str(sbuf, -12.34, 4, 'e'), Str{"-1.2340e+01"});
  panicking::assert_eq(num::flt2str(sbuf, 12345678.9, 4, 'e'), Str{"1.2346e+07"});

  // inf
  volatile double zero = 0.0;
  panicking::assert_eq(num::flt2str(sbuf, +1.0 / zero), Str{"inf"});
  panicking::assert_eq(num::flt2str(sbuf, -1.0 / zero), Str{"-inf"});
}

}  // namespace sfc::num::test
