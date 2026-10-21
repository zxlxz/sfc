#include <unistd.h>
#include <pthread.h>

#ifdef __linux__
#include <sys/syscall.h>
#endif

#include "sfc/sys/posix/thread.h"

namespace sfc::sys::posix {

struct ThreadData {
  Box<void()> _fun;
};

static void* thread_start(void* p) {
  auto data = ptr::cast<ThreadData>(p);
  auto func = mem::move(data->_fun);
  delete data;
  (func)();
  return nullptr;
}

void Thread::join() {
  // just join, don't change thrd, the caller should handle it
  const auto err = ::pthread_join(_handle, nullptr);
  sfc::assert_(err == 0, "Thread::join: pthread_join failed, err={}", err);
}

void Thread::detach() {
  // just detach, don't change thrd, the caller should handle it
  const auto err = ::pthread_detach(_handle);
  sfc::assert_(err == 0, "Thread::detach: pthread_detach failed, err={}", err);
}

auto Thread::new_(usize stack_size, Box<void()> obj) -> Thread {
  auto attr = pthread_attr_t{};
  ::pthread_attr_init(&attr);
  ::pthread_attr_setstacksize(&attr, stack_size);

  auto data = new ThreadData{mem::move(obj)};

  auto thr = pthread_t{};
  const auto err = ::pthread_create(&thr, &attr, &thread_start, data);
  ::pthread_attr_destroy(&attr);

  if (err != 0) {
    delete data;
  }

  return Thread{thr};
}

auto this_thread::id() noexcept -> u32 {
#ifdef __APPLE__
  auto tid = uint64_t{0};
  ::pthread_threadid_np(nullptr, &tid);
#else
  const auto tid = ::syscall(SYS_gettid);
#endif
  return u32(tid);
}

auto this_thread::set_name(const char* name) -> bool {
#ifdef __APPLE__
  const auto err = ::pthread_setname_np(name);
#else
  const auto thr = ::pthread_self();
  const auto err = ::pthread_setname_np(thr, name);
#endif
  return err == 0;
}

void this_thread::yield() {
  ::sched_yield();
}

void this_thread::sleep(time::Duration dur) noexcept {
  if (dur._nanos <= time::NANOS_PER_MILLI) {
    ::sched_yield();
    return;
  }

  const auto ts = ::timespec{
      .tv_sec = time_t(dur.as_secs()),
      .tv_nsec = long{dur.subsec_nanos()},
  };

  (void)::nanosleep(&ts, nullptr);
}

}  // namespace sfc::sys::posix
