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

}  // namespace sfc::time
