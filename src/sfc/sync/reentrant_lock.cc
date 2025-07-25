#include "sfc/sync/reentrant_lock.h"

#include "sfc/sync/atomic.h"
#include "sfc/sys/sync.h"
#include "sfc/thread.h"

namespace sfc::sync {

using sys::sync::mutex_t;
using thread::Thread;

struct ReentrantLock::Inn {
  Atomic<thread::raw_t> _owner{{}};
  Atomic<int> _count{0};
  mutex_t _mutex{};

 public:
  void lock() {
    const auto cur_thrd = Thread::current()._raw;

    if (cur_thrd == _owner.load()) {
      _count.fetch_add(1);
      return;
    }

    sys::sync::lock(_mutex);
    _owner.store(cur_thrd);
    _count.store(1);
  }

  void unlock() {
    const auto cur_thrd = Thread::current()._raw;

    if (cur_thrd != _owner.load()) {
      panicking::panic("ReentrantLock::unlock: not owner of the lock");
      return;
    }

    if (_count.fetch_sub(1) == 1) {
      sys::sync::unlock(_mutex);
      _owner.store({});
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
