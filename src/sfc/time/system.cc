#include "sfc/time/system.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto System::now() -> System {
  const auto nanos = sys_imp::system_now();
  return System{nanos};
}

auto System::elapsed() const -> Duration {
  const auto now = System::now();
  const auto res = now - *this;
  return res;
}

auto System::operator-(const System& rhs) const -> Duration {
  const auto nanos = num::saturating_sub(_nanos, rhs._nanos);
  return Duration::from_nanos(nanos);
}

auto System::operator+(const Duration& dur) const -> System {
  const auto nanos = num::saturating_add(_nanos, dur.as_nanos());
  return System{nanos};
}

auto System::operator-(const Duration& dur) const -> System {
  const auto nanos = num::saturating_sub(_nanos, dur.as_nanos());
  return System{nanos};
}

auto System::operator+=(const Duration& dur) -> System& {
  const auto nanos = num::saturating_add(_nanos, dur.as_nanos());
  _nanos = nanos;
  return *this;
}

auto System::operator-=(const Duration& dur) -> System& {
  const auto nanos = num::saturating_sub(_nanos, dur.as_nanos());
  _nanos = nanos;
  return *this;
}

}  // namespace sfc::time
