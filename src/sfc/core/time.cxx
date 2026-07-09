#include "sfc/test.h"
#include "sfc/io.h"

namespace sfc::time::test {

SFC_TEST(duration_from) {
  const auto a = Duration::from_secs(1);
  sfc::assert_eq(a._nanos, 1000000000U);

  const auto b = Duration::from_millis(1);
  sfc::assert_eq(b._nanos, 1000000U);

  const auto c = Duration::from_micros(1);
  sfc::assert_eq(c._nanos, 1000U);

  const auto d = Duration::from_nanos(1);
  sfc::assert_eq(d._nanos, 1U);

  const auto e = Duration::from_secs_f32(1.5F);
  sfc::assert_eq(e._nanos, 1500000000U);

  const auto f = Duration::from_secs_f64(1.5);
  sfc::assert_eq(f._nanos, 1500000000U);
}

SFC_TEST(duration_as) {
  const auto t = Duration::from_millis(15);
  sfc::assert_eq(t.as_secs(), 0U);
  sfc::assert_eq(t.as_nanos(), 15000000U);
  sfc::assert_eq(t.as_millis(), 15U);
  sfc::assert_eq(t.as_micros(), 15000U);

  sfc::assert_flt_eq(t.as_secs_f32(), 0.015, 1e-6);
  sfc::assert_flt_eq(t.as_secs_f64(), 0.015, 1e-10);
}

SFC_TEST(duration_subsec) {
  const auto t = Duration::from_millis(15);
  sfc::assert_eq(t.subsec_millis(), 15U);
  sfc::assert_eq(t.subsec_micros(), 15000U);
  sfc::assert_eq(t.subsec_nanos(), 15000000U);
}

SFC_TEST(duration_cmp) {
  const auto a = Duration::from_millis(15);
  const auto b = Duration::from_millis(20);

  sfc::assert_eq(a < b, true);
  sfc::assert_eq(a > b, false);
  sfc::assert_eq(a <= b, true);
  sfc::assert_eq(a >= b, false);

  sfc::assert_eq(b < a, false);
  sfc::assert_eq(b > a, true);
  sfc::assert_eq(b <= a, false);
  sfc::assert_eq(b >= a, true);

  sfc::assert_eq(a == b, false);
  sfc::assert_eq(a != b, true);
}

SFC_TEST(duration_fmt) {
  io::println("1.5s => {}", Duration::from_secs_f32(1.5F));
  io::println("15ms => {}", Duration::from_millis(15));
  io::println("15us => {}", Duration::from_micros(15));
  io::println("15ns => {}", Duration::from_nanos(15));
}

}  // namespace sfc::time::test
