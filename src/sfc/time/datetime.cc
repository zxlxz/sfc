#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/time.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/time.inl"
#endif

#include "sfc/time/datetime.h"

namespace sfc::time {

auto DateTime::from_utc(SysTime sys_time) noexcept -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();
  const auto datetime = sys::DateTime::from_utc(secs);

  const auto res = DateTime{
      .year = datetime.year,
      .month = datetime.month,
      .day = datetime.day,
      .hour = datetime.hour,
      .minute = datetime.minute,
      .second = datetime.second,
      .nanos = nanos,
  };
  return res;
}

auto DateTime::from_local(SysTime sys_time) noexcept -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.subsec_nanos();
  const auto datetime = sys::DateTime::from_local(secs);

  const auto res = DateTime{
      .year = datetime.year,
      .month = datetime.month,
      .day = datetime.day,
      .hour = datetime.hour,
      .minute = datetime.minute,
      .second = datetime.second,
      .nanos = nanos,
  };
  return res;
}

}  // namespace sfc::time
