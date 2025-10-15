#include "sfc/time.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto nanos = sys_imp::instant_now();
  return Instant{nanos};
}

auto SystemTime::now() -> SystemTime {
  const auto nanos = sys_imp::system_now();
  return SystemTime{nanos};
}

auto DateTime::from_utc(SystemTime sys_time) -> DateTime {
  auto res = DateTime{};
  sys_imp::make_utc(sys_time._nanos, res);
  return res;
}

auto DateTime::from_local(SystemTime sys_time) -> DateTime {
  auto res = DateTime{};
  sys_imp::make_local(sys_time._nanos, res);
  res.nanos = sys_time._nanos % NANOS_PER_SEC;
  return res;
}

}  // namespace sfc::time
