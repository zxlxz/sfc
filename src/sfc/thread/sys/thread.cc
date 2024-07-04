#include "thread.h"

#include <errno.h>
#include <pthread.h>

namespace sfc::thread::sys {

static auto thread_start(void* raw) -> void* {
  auto box = Box<void()>::from_raw(raw);
  box();
  return nullptr;
}

auto Thread::create(Box<void()> fun) -> Thread {
  auto res = Thread{};

  const auto ptr = mem::move(fun).into_raw();
  const auto ret = ::pthread_create(&res._id, nullptr, thread_start, ptr);
  if (ret != 0) {
    auto tmp = Box<void()>::from_raw(ptr);
    (void)tmp;
  }
  return res;
}

auto Thread::id() const -> u64 {
  return _id;
}

void Thread::join() {
  (void)::pthread_join(_id, nullptr);
}

void sleep(time::Duration dur) {
  struct timespec ts = {
      static_cast<time_t>(dur.as_secs()),
      static_cast<time_t>(dur.subsec_nanos()),
  };

  for (;;) {
    const auto ret = ::nanosleep(&ts, &ts);
    if (ret != -1 || errno == EINTR) {
      break;
    }
  }
}

}  // namespace sfc::thread::sys
