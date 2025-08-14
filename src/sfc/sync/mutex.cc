#include "mutex.h"

#include "sfc/sys/sync.h"
#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn {
  sys_imp::mutex_t _raw;

 public:
  Inn() {
    sys_imp::init(_raw);
  }

  ~Inn() {
    sys_imp::drop(_raw);
  }

  Inn(const Inn&) = delete;
  Inn& operator=(const Inn&) = delete;
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
  using tid_t = sys::thread::tid_t;

  Atomic<tid_t> _owner{{}};
  Atomic<int> _count{0};
  sys_imp::mutex_t _mutex{};

 public:
  Inn() {
    sys_imp::init(_mutex);
  }

  ~Inn() {
    sys_imp::drop(_mutex);
  }

  Inn(const Inn&) = delete;

  void lock() {
    const auto cur_thrd = sys::thread::current_id();

    if (cur_thrd == _owner.load()) {
      _count.fetch_add(1);
      return;
    }

    sys::sync::lock(_mutex);
    _count.store(1);
    _owner.store(cur_thrd);
  }

  void unlock() {
    const auto cur_thrd = sys::thread::current_id();

    if (cur_thrd != _owner.load()) {
      // not owner of the lock, so do nothing
      return;
    }

    if (_count.fetch_sub(1) == 1) {
      _owner.store(tid_t{});
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
  _inn ? _inn->unlock() : void();
}

}  // namespace sfc::sync
