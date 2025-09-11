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

  void lock() {
    sys_imp::lock(_raw);
  }

  void unlock() {
    sys_imp::unlock(_raw);
  }
};

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
  Inn& operator=(const Inn&) = delete;

  void lock() {
    const auto cur_thrd = sys::thread::current_id();

    if (cur_thrd == _owner.load()) {
      _count.fetch_add(1);
      return;
    }

    sys::sync::lock(_mutex);
    _owner.store(cur_thrd);
    _count.store(1);
  }

  auto unlock() -> bool {
    const auto cur_thrd = sys::thread::current_id();

    if (cur_thrd != _owner.load()) {
      return false;
    }

    if (_count.fetch_sub(1) == 1) {
      _owner.store(tid_t{});
      sys::sync::unlock(_mutex);
    }
    return true;
  }
};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> Guard {
  panicking::expect(_inn, "Mutex::lock: on a dropped object");
  return Guard{*_inn};
}

Mutex::Guard::Guard(Inn& mtx) : _mtx{&mtx} {
  _mtx->lock();
}

Mutex::Guard::~Guard() noexcept {
  if (!_mtx) {
    return;
  }
  _mtx->unlock();
}

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() noexcept {}

auto ReentrantLock::lock() -> Guard {
  panicking::expect(_inn, "ReentrantLock::lock: on a dropped object");
  return Guard{*_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) : _mtx{&inn} {
  _mtx->lock();
}

ReentrantLock::Guard::~Guard() noexcept {
  if (!_mtx) {
    return;
  }
  _mtx->unlock();
}

}  // namespace sfc::sync
