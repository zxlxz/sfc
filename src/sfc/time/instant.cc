#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/time.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/time.inl"
#endif

#include "sfc/time/instant.h"

namespace sfc::time {

auto Instant::now() noexcept -> Instant {
  const auto ts = sys::Instant::now();
  return Instant{ts.nanos()};
}

auto Instant::elapsed() const noexcept -> Duration {
  const auto now = Instant::now();
  const auto dur = num::saturating_sub(now._nanos, _nanos);
  return Duration::from_nanos(dur);
}

auto Instant::duration_since(Instant earlier) const noexcept -> Duration {
  const auto dur = num::saturating_sub(_nanos, earlier._nanos);
  return Duration::from_nanos(dur);
}

auto Instant::operator==(const Instant& other) const noexcept -> bool {
  return _nanos == other._nanos;
}

auto Instant::operator+(const Duration& dur) const noexcept -> Instant {
  const auto ns = _nanos + dur.as_nanos();
  return Instant{ns};
}

auto Instant::operator-(const Duration& dur) const noexcept -> Instant {
  const auto ns = num::saturating_sub(_nanos, dur.as_nanos());
  return Instant{ns};
}

}  // namespace sfc::time
