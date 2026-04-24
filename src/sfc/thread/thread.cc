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
  sys::sleep(dur);
}

void sleep_ms(u32 ms) {
  const auto dur = time::Duration::from_millis(ms);
  sys::sleep(dur);
}

}  // namespace sfc::thread
