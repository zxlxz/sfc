#include "sfc/time/datetime.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto DateTime::from_utc(SysTime sys_time) noexcept -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();

  auto res = DateTime{.nanos = nanos};
  sys_imp::make_utc(secs, res);
  return res;
}

auto DateTime::from_local(SysTime sys_time) noexcept -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();

  auto res = DateTime{.nanos = nanos};
  sys_imp::make_local(secs, res);
  return res;
}

}  // namespace sfc::time
