#include "sfc/chrono.h"

#include "sfc/sys/time.h"

namespace sfc::chrono {

namespace sys_imp = sys::time;
using time::System;

auto DateTime::from_utc(const System& sys_time) -> DateTime {
  const auto secs = sys_time._nanos / time::NANOS_PER_SEC;
  return sys_imp::make_utc<DateTime>(secs);
}

auto DateTime::from_local(const System& sys_time) -> DateTime {
  const auto secs = sys_time._nanos / time::NANOS_PER_SEC;
  return sys_imp::make_local<DateTime>(secs);
}

}  // namespace sfc::chrono
