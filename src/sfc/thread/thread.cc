#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/thread.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/thread.inl"
#endif

#include "sfc/thread/thread.h"

namespace sfc::thread {

auto current() -> Thread {
  const auto id = sys::Thread::current();
  return Thread{id};
}

auto current_id() -> u32 {
  const auto id = sys::Thread::current();
  return id;
}

void yield_now() {
  sys::Thread::yield_now();
}

void sleep(time::Duration dur) {
  const auto millis = static_cast<u32>(dur.as_millis());
  sys::sleep_ms(millis);
}

void sleep_ms(u32 ms) {
  sys::sleep_ms(ms);
}

}  // namespace sfc::thread
