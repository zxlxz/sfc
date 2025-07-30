#include "sfc/time.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto nanos = sys_imp::instant_now();
  return Instant{nanos};
}

auto System::now() -> System {
  const auto nanos = sys_imp::system_now();
  return System{nanos};
}

}  // namespace sfc::time
