#include <unistd.h>
#include <pthread.h>

#ifdef __linux__
#include <sys/syscall.h>
#endif

#include "sfc/sys/posix/thread.h"

namespace sfc::sys::posix {

struct ThreadAttr {
  pthread_attr_t _raw;

  ThreadAttr() {
    ::pthread_attr_init(&_raw);
  }

  ~ThreadAttr() {
    ::pthread_attr_destroy(&_raw);
  }

  void set_stack_size(size_t stack_size) {
    if (stack_size == 0) return;
    ::pthread_attr_setstacksize(&_raw, stack_size);
  }
};

auto ThisThread::id() noexcept -> u32 {
#ifdef __APPLE__
  auto tid = uint64_t{0};
  ::pthread_threadid_np(nullptr, &tid);
#else
  const auto tid = ::syscall(SYS_gettid);
#endif
  return u32(tid);
}

auto ThisThread::set_name(const char* name) -> bool {
#ifdef __APPLE__
  const auto err = ::pthread_setname_np(name);
#else
  const auto thr = ::pthread_self();
  const auto err = ::pthread_setname_np(thr, name);
#endif
  return err == 0;
}

void ThisThread::yield() {
  ::sched_yield();
}

void ThisThread::sleep(time::Duration dur) noexcept {
  if (dur._nanos <= time::NANOS_PER_MILLI) {
    ::sched_yield();
    return;
  }

  const auto secs = num::cast_signed(dur.as_secs());
  const auto nanos = num::cast_signed(dur.subsec_nanos());
  const auto ts = ::timespec{
      .tv_sec = time_t{secs},
      .tv_nsec = long{nanos},
  };

  (void)::nanosleep(&ts, nullptr);
}

auto Thread::is_valid() const -> bool {
  return _handle != 0;
}

void Thread::join() {
  if (_handle == 0) {
    return;
  }

  // just join, don't change thrd, the caller should handle it
  const auto err = ::pthread_join(_handle, nullptr);
  sfc::assert_(err == 0, "Thread::join: pthread_join failed, err={}", err);
}

void Thread::detach() {
  if (_handle == 0) {
    return;
  }

  // just detach, don't change thrd, the caller should handle it
  const auto err = ::pthread_detach(_handle);
  sfc::assert_(err == 0, "Thread::detach: pthread_detach failed, err={}", err);
}

struct ThreadData {
  Box<void()> _fun;
};

static void* start_routine(void* p) {
  auto data = ptr::cast<ThreadData>(p);
  auto func = mem::move(data->_fun);
  delete data;

  (func)();
  return nullptr;
}

auto Thread::spawn(usize stack_size, Box<void()> obj) -> Thread {
  auto attr = pthread_attr_t{};
  ::pthread_attr_init(&attr);
  ::pthread_attr_setstacksize(&attr, stack_size);

  auto data = new ThreadData{mem::move(obj)};

  auto thr = pthread_t{};
  const auto err = ::pthread_create(&thr, &attr, &start_routine, data);
  ::pthread_attr_destroy(&attr);

  if (err != 0) {
    delete data;
  }

  return Thread{thr};
}

}  // namespace sfc::sys::posix
