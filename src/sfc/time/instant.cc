#include "sfc/time/instant.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto nanos = sys_imp::instant_now();
  return Instant{nanos};
}

auto Instant::elapsed() const -> Duration {
  const auto now = Instant::now();
  const auto res = now - *this;
  return res;
}

}  // namespace sfc::time
