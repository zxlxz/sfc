#include "sfc/sys/thread.h"

#include "sfc/thread/thread.h"

namespace sfc::thread {

auto current() -> Thread {
  const auto tid = thread::current_id();
  return Thread{tid};
}

auto current_id() -> u32 {
  const auto tid = sys::this_thread::id();
  return tid;
}

void yield_now() {
  sys::this_thread::yield();
}

void sleep(time::Duration dur) {
  sys::this_thread::sleep(dur);
}

void sleep_ms(u32 ms) {
  const auto dur = time::Duration::from_millis(ms);
  sys::this_thread::sleep(dur);
}

void sleep_until(time::Instant deadline) {
  const auto now = time::Instant::now();
  const auto dur = deadline.duration_since(now);
  sys::this_thread::sleep(dur);
}
}  // namespace sfc::thread
