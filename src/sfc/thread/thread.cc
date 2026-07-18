#include "sfc/thread/thread.h"
#include "sfc/sys.h"

namespace sfc::thread {

auto current() -> Thread {
  const auto tid = thread::current_id();
  return Thread{tid};
}

auto current_id() -> u32 {
  static thread_local const auto tid = sys::ThisThread::id();
  return tid;
}

void yield_now() {
  sys::ThisThread::yield();
}

void sleep(time::Duration dur) {
  sys::ThisThread::sleep(dur);
}

void sleep_ms(u32 ms) {
  const auto dur = time::Duration::from_millis(ms);
  sys::ThisThread::sleep(dur);
}

}  // namespace sfc::thread
