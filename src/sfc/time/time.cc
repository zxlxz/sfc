#include "sfc/time.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto nanos = sys_imp::instant_now();
  return Instant{nanos};
}

auto System::now() -> System {
  const auto nanos = sys_imp::system_now();
  return System{nanos};
}

auto DateTime::from_utc(const System& sys_time) -> DateTime {
  const auto secs = sys_time._nanos / time::NANOS_PER_SEC;
  const auto nanos = sys_time._nanos % time::NANOS_PER_SEC;

  auto res = sys_imp::make_utc<DateTime>(secs);
  res.nanos = nanos;
  return res;
}

auto DateTime::from_local(const System& sys_time) -> DateTime {
  const auto secs = sys_time._nanos / time::NANOS_PER_SEC;
  const auto nanos = sys_time._nanos % time::NANOS_PER_SEC;
  auto res = sys_imp::make_local<DateTime>(secs);
  res.nanos = nanos;
  return res;
}

}  // namespace sfc::time
