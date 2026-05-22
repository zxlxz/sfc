#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/time.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/time.inl"
#endif

#include "sfc/time/systime.h"

namespace sfc::time {

auto SystemTime::now() noexcept -> SystemTime {
  const auto sys_imp = sys::SystemTime::now();
  return SystemTime{sys_imp.as_micros()};
}

auto SystemTime::elapsed() const noexcept -> Duration {
  const auto now = SystemTime::now();
  const auto dur = num::saturating_sub(now._micros, _micros);
  return Duration::from_micros(dur);
}

auto SystemTime::duration_since(SystemTime earlier) const noexcept -> Duration {
  const auto dur = num::saturating_sub(_micros, earlier._micros);
  return Duration::from_micros(dur);
}

auto SystemTime::as_secs() const noexcept -> u64 {
  return _micros / MICROS_PER_SEC;
}

auto SystemTime::subsec_nanos() const noexcept -> u32 {
  const auto nanos = _micros % MICROS_PER_SEC * NANOS_PER_MICRO;
  return static_cast<u32>(nanos);
}

auto SystemTime::subsec_micros() const noexcept -> u32 {
  const auto micros = _micros % MICROS_PER_SEC;
  return static_cast<u32>(micros);
}

auto SystemTime::subsec_millis() const noexcept -> u32 {
  const auto millis = _micros % MICROS_PER_SEC / MICROS_PER_MILLI;
  return static_cast<u32>(millis);
}

auto SystemTime::operator==(const SystemTime& other) const noexcept -> bool {
  return _micros == other._micros;
}

auto SystemTime::operator+(const Duration& dur) const noexcept -> SystemTime {
  const auto us = _micros + dur.as_micros();
  return SystemTime{us};
}

auto SystemTime::operator-(const Duration& dur) const noexcept -> SystemTime {
  const auto us = num::saturating_sub(_micros, dur.as_micros());
  return SystemTime{us};
}

}  // namespace sfc::time
