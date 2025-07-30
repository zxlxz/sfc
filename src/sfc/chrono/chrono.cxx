#include "sfc/chrono.h"

#include "sfc/test.h"

namespace sfc::chrono::test {

SFC_TEST(datetime_utc_local) {
  auto sys_time = time::System::now();
  auto dt_utc = DateTime::from_utc(sys_time);
  auto dt_local = DateTime::from_local(sys_time);

  panicking::assert_true(dt_utc.year() >= 2025);
  panicking::assert_true(dt_utc.month() >= 1 && dt_utc.month() <= 12);
  panicking::assert_true(dt_utc.day() >= 1 && dt_utc.day() <= 31);

  panicking::assert_true(dt_local.year() >= 2025);
  panicking::assert_true(dt_local.month() >= 1 && dt_local.month() <= 12);
  panicking::assert_true(dt_local.day() >= 1 && dt_local.day() <= 31);

  panicking::assert_true(dt_local.year() >= dt_utc.year());
  panicking::assert_true(dt_local.month() >= dt_utc.month());
  panicking::assert_true(dt_local.day() >= dt_utc.day());
  panicking::assert_true(dt_local.hour() > dt_utc.hour());
  panicking::assert_true(dt_local.minute() >= dt_utc.minute());
  panicking::assert_true(dt_local.second() >= dt_utc.second());
}

}  // namespace sfc::chrono::test
