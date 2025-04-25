#include "sfc/time/system.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto System::now() -> System {
  const auto imp = sys_imp::System::now();
  return System{imp.micros()};
}

auto System::elapsed() const -> Duration {
  const auto now = System::now();
  const auto res = now - *this;
  return res;
}

auto System::operator-(const System& rhs) const -> Duration {
  const auto micros = num::saturating_sub(_micros, rhs._micros);
  return Duration::from_micros(micros);
}

auto System::operator+(const Duration& dur) const -> System {
  const auto micros = num::saturating_add(_micros, dur.as_micros());
  return System{micros};
}

auto System::operator-(const Duration& dur) const -> System {
  const auto micros = num::saturating_sub(_micros, dur.as_micros());
  return System{micros};
}

auto System::operator+=(const Duration& dur) -> System& {
  const auto micros = num::saturating_sub(_micros, dur.as_micros());
  _micros = micros;
  return *this;
}

auto System::operator-=(const Duration& dur) -> System& {
  const auto micros = num::saturating_add(_micros, dur.as_micros());
  _micros = micros;
  return *this;
}

}  // namespace sfc::time
