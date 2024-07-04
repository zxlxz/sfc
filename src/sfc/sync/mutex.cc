#include "mutex.h"

#include "sfc/sys/sync.h"
#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn {
  sys_imp::mutex_t _raw;
};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> Guard {
  panicking::assert(_inn, "Mutex::lock: on a dropped object");
  return Guard{*_inn};
}

Mutex::Guard::Guard(Inn& mtx) : _mtx{&mtx} {
  sys_imp::lock(mtx._raw);
}

Mutex::Guard::~Guard() {
  if (!_mtx) {
    return;
  }
  sys_imp::unlock(_mtx->_raw);
}

struct ReentrantLock::Inn {
  using thrd_t = sys::thread::thrd_t;

  Atomic<thrd_t> _owner{{}};
  Atomic<int> _count{0};
  sys_imp::mutex_t _mutex{};

 public:
  void lock() {
    const auto cur_thrd = sys::thread::current();

    if (cur_thrd == _owner.load<Ordering::Acquire>()) {
      _count.fetch_add<Ordering::Relaxed>(1);
      return;
    }

    sys::sync::lock(_mutex);
    _owner.store(cur_thrd);
    _count.store(1);
  }

  void unlock() {
    const auto cur_thrd = sys::thread::current();

    if (cur_thrd != _owner.load<Ordering::Acquire>()) {
      // not owner of the lock, so do nothing
      return;
    }

    if (_count.fetch_sub<Ordering::Relaxed>(1) == 1) {
      _owner.store(thrd_t{});
      sys::sync::unlock(_mutex);
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
