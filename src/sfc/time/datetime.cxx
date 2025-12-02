#include "sfc/time.h"

#include "sfc/test.h"

namespace sfc::time::test {

SFC_TEST(datetime_utc) {
  auto sys_time = time::SysTime::now();
  auto dt_utc = DateTime::from_utc(sys_time);

  panicking::expect(dt_utc.year >= 2025);
  panicking::expect(dt_utc.month >= 1 && dt_utc.month <= 12);
  panicking::expect(dt_utc.day >= 1 && dt_utc.day <= 31);
}

SFC_TEST(datetime_local) {
  auto sys_time = time::SysTime::now();
  auto dt_local = DateTime::from_local(sys_time);

  panicking::expect(dt_local.year >= 2025);
  panicking::expect(dt_local.month >= 1 && dt_local.month <= 12);
  panicking::expect(dt_local.day >= 1 && dt_local.day <= 31);
}

}  // namespace sfc::chrono::test
\
