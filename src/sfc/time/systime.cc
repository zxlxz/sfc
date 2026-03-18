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

}  // namespace sfc::time
