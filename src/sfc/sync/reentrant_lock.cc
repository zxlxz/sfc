#include "sfc/sync/reentrant_lock.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

struct ReentrantLock::Inn {
  static constexpr auto NULL_OWNER = static_cast<u64>(-1);

  sys_imp::Mutex _mutex{};
  u64 _owner{NULL_OWNER};
  u32 _count{0};

 public:
  void lock() {
    const auto thrd = sys_imp::Thread::get_tid();

    const auto cur_owner = __atomic_load_n(&_owner, __ATOMIC_SEQ_CST);
    if (thrd == cur_owner) {
      __atomic_fetch_add(&_count, 1, __ATOMIC_SEQ_CST);
      return;
    }

    _mutex.lock();
    __atomic_store_n(&_owner, thrd, __ATOMIC_SEQ_CST);
    __atomic_store_n(&_count, 1, __ATOMIC_SEQ_CST);
  }

  void unlock() {
    const auto thrd = sys_imp::Thread::get_tid();

    const auto cur_owner = __atomic_load_n(&_owner, __ATOMIC_SEQ_CST);
    if (thrd != cur_owner) {
      panicking::panic("ReentrantLock::unlock: not owner of the lock");
      return;
    }

    const auto old_count = __atomic_load_n(&_count, __ATOMIC_SEQ_CST);
    if (old_count == 1) {
      __atomic_store_n(&_owner, NULL_OWNER, __ATOMIC_SEQ_CST);
      _mutex.unlock();
    }
  }
};

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() {}

auto ReentrantLock::lock() -> Guard {
  panicking::assert(_inn, "ReentrantLock::lock: cannot lock on a dropped object");
  return Guard{*_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) : _inn{&inn} {
  _inn->lock();
}

ReentrantLock::Guard::~Guard() noexcept {
  _inn->unlock();
}

}  // namespace sfc::sync
