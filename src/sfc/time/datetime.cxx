#include "sfc/time.h"

#include "sfc/test/test.h"

namespace sfc::time::test {

SFC_TEST(datetime_utc) {
  auto sys_time = time::SystemTime::now();
  auto dt_utc = DateTime::from_utc(sys_time);

  sfc::assert_ge(dt_utc.year, 2025);

  sfc::assert_ge(dt_utc.month, 1);
  sfc::assert_le(dt_utc.month, 12);

  sfc::assert_ge(dt_utc.day, 1);
  sfc::assert_le(dt_utc.day, 31);
}

SFC_TEST(datetime_local) {
  auto sys_time = time::SystemTime::now();
  auto dt_local = DateTime::from_local(sys_time);

  sfc::assert_ge(dt_local.year, 2025);

  sfc::assert_ge(dt_local.month, 1);
  sfc::assert_le(dt_local.month, 12);

  sfc::assert_ge(dt_local.day, 1);
  sfc::assert_le(dt_local.day, 31);
}

}  // namespace sfc::time::test
