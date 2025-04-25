#include "sfc/time/instant.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto imp = sys_imp::Instant::now();
  return Instant{imp.nanos()};
}

auto Instant::elpased() const -> Duration {
  const auto now = Instant::now();
  const auto res = now - *this;
  return res;
}

auto Instant::duration_since(const Instant& earlier) const -> Duration {
  const auto res = *this - earlier;
  return res;
}

auto Instant::operator==(const Instant& other) const -> bool {
  return _nanos == other._nanos;
}

auto Instant::operator<(const Instant& other) const -> bool {
  return _nanos < other._nanos;
}

auto Instant::operator<=(const Instant& other) const -> bool {
  return _nanos <= other._nanos;
}

auto Instant::operator-(const Instant& rhs) const -> Duration {
  const auto nanos = num::saturating_sub(_nanos, rhs._nanos);
  return Duration::from_nanos(nanos);
}

auto Instant::operator+(const Duration& dur) const -> Instant {
  const auto nanos = num::saturating_add(_nanos, dur.as_nanos());
  return Instant{nanos};
}

auto Instant::operator-(const Duration& dur) const -> Instant {
  const auto nanos = num::saturating_sub(_nanos, dur.as_nanos());
  return Instant{nanos};
}

void Instant::operator+=(const Duration& dur) {
  *this = *this + dur;
}

void Instant::operator-=(const Duration& dur) {
  *this = *this - dur;
}

}  // namespace sfc::time
