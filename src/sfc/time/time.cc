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
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();

  auto res = DateTime{.nanos = nanos};
  sys_imp::make_utc(secs, res);
  return res;
}

auto DateTime::from_local(SystemTime sys_time) -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();

  auto res = DateTime{.nanos = nanos};
  sys_imp::make_local(secs, res);
  return res;
}

}  // namespace sfc::time
