#include "sfc/sync/reentrant_lock.h"

#include "sfc/sys/sync.h"
#include "sfc/sys/thread.h"

namespace sfc::sync {

struct ReentrantLock::Inn {
  sys::sync::mutex_t _mutex{};
  sys::thread::thrd_t _owner{};
  int _count{0};

 public:
  void lock() {
    const auto thrd = sys::thread::current();

    const auto cur_owner = __atomic_load_n(&_owner, __ATOMIC_SEQ_CST);
    if (thrd == cur_owner) {
      __atomic_fetch_add(&_count, 1, __ATOMIC_SEQ_CST);
      return;
    }

    sys::sync::lock(_mutex);
    __atomic_store_n(&_owner, thrd, __ATOMIC_SEQ_CST);
    __atomic_store_n(&_count, 1, __ATOMIC_SEQ_CST);
  }

  void unlock() {
    const auto thrd = sys::thread::current();

    const auto cur_owner = __atomic_load_n(&_owner, __ATOMIC_SEQ_CST);
    if (thrd != cur_owner) {
      panicking::panic("ReentrantLock::unlock: not owner of the lock");
      return;
    }

    const auto old_cnt = __atomic_fetch_add(&_count, -1, __ATOMIC_SEQ_CST);
    if (old_cnt == 1) {
      sys::sync::unlock(_mutex);
      __atomic_store_n(&_owner, sys::thread::thrd_t{}, __ATOMIC_SEQ_CST);
    }
  }
};

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() {}

auto ReentrantLock::lock() -> Guard {
  panicking::assert(_inn, "ReentrantLock::lock: on a dropped object");
  return Guard{*_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) : _inn{&inn} {
  _inn->lock();
}

ReentrantLock::Guard::~Guard() noexcept {
  _inn->unlock();
}

}  // namespace sfc::sync
