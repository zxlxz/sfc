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

}  // namespace sfc::time
