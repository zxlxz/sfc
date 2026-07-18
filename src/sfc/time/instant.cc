#define _SFC_SYS_TIME_
#include "sfc/sys.h"
#include "sfc/time/instant.h"

namespace sfc::time {

auto Instant::now() noexcept -> Instant {
  const auto sys_time = sys::Instant::now();
  const auto nanos = sys_time.sec * NANOS_PER_SEC + sys_time.nsec;
  return Instant{nanos};
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

auto Instant::operator==(Instant other) const noexcept -> bool {
  return _nanos == other._nanos;
}

auto Instant::operator<=>(Instant other) const noexcept -> int {
  if (_nanos < other._nanos) return -1;
  if (_nanos > other._nanos) return +1;
  return 0;
}

auto Instant::operator+(Duration dur) const noexcept -> Instant {
  const auto ns = _nanos + dur.as_nanos();
  return Instant{ns};
}

auto Instant::operator-(Duration dur) const noexcept -> Instant {
  const auto ns = num::saturating_sub(_nanos, dur.as_nanos());
  return Instant{ns};
}

auto Instant::operator-(Instant other) const noexcept -> Duration {
  const auto dur = num::saturating_sub(_nanos, other._nanos);
  return Duration::from_nanos(dur);
}

void Instant::fmt(fmt::Formatter& f) const {
  f.write_fmt("{}ns", _nanos);
}

}  // namespace sfc::time
