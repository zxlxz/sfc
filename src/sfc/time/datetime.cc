#include "sfc/time/datetime.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto NaiveTime::from_hms(u32 hour, u32 min, u32 sec) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  return {._secs = secs, ._nanos = 0};
}

auto NaiveTime::from_hms_micro(u32 hour, u32 min, u32 sec, u32 micros) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  const auto nanos = static_cast<u32>(NANOS_PER_MICRO * micros);
  return {._secs = secs, ._nanos = nanos};
}

auto NaiveTime::from_hms_milli(u32 hour, u32 min, u32 sec, u32 millis) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  const auto nanos = NANOS_PER_MILLI * millis;
  return {._secs = secs, ._nanos = nanos};
}

auto NaiveTime::from_hms_nano(u32 hour, u32 min, u32 sec, u32 nano) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  return {._secs = secs, ._nanos = nano};
}

auto DateTime::from(const System& sys_time) -> DateTime {
  const auto secs = sys_time.secs();
  const auto nanos = sys_time.sub_nanos();
  const auto dt = sys_imp::DateTime::from_secs(secs);

  const auto date = NaiveDate::from_ymd(dt.year, dt.month, dt.mday);
  const auto time = NaiveTime::from_hms_nano(dt.hour, dt.min, dt.sec, nanos);
  return {date, time};
}

auto DateTime::now() -> DateTime {
  const auto sys_time = System::now();
  return DateTime::from(sys_time);
}

}  // namespace sfc::time
