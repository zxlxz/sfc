#include "sfc/sync/mutex.h"
#include "sfc/sync/atomic.h"

#include "sfc/sys/sync.h"
#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

using tid_t = sys_imp::tid_t;
using mtx_t = sys_imp::mtx_t;

struct Mutex::Inn {
  mtx_t _raw;

 public:
  Inn() noexcept {
    sys_imp::mtx_init(_raw);
  }

  ~Inn() noexcept {
    sys_imp::mtx_destroy(_raw);
  }

  void lock() noexcept {
    sys_imp::mtx_lock(_raw);
  }

  void unlock() noexcept {
    sys_imp::mtx_unlock(_raw);
  }

  bool try_lock() noexcept {
    return sys_imp::mtx_trylock(_raw);
  }
};

struct ReentrantLock::Inn {
  mtx_t _raw = {};
  Atomic<tid_t> _tid = {};
  Atomic<i32> _cnt = {0};

 public:
  Inn() noexcept {
    sys_imp::mtx_init(_raw);
  }

  ~Inn() noexcept {
    sys_imp::mtx_destroy(_raw);
  }

  void lock() noexcept {
    const auto tid = sys_imp::get_tid();

    if (_tid.load(sync::Ordering::Acquire) == tid) {
      _cnt.fetch_add(1, sync::Ordering::Relaxed);
      return;
    }

    sys_imp::mtx_lock(_raw);
    _tid.store(tid, sync::Ordering::Release);
    _cnt.store(1, sync::Ordering::Relaxed);
  }

  bool try_lock() noexcept {
    const auto tid = sys_imp::get_tid();

    if (tid == _tid.load(sync::Ordering::Acquire)) {
      _cnt.fetch_add(1, sync::Ordering::Relaxed);
      return true;
    }

    if (!sys_imp::mtx_trylock(_raw)) {
      return false;
    }

    _tid.store(tid, sync::Ordering::Release);
    _cnt.store(1, sync::Ordering::Relaxed);
    return true;
  }

  bool unlock() noexcept {
    const auto tid = sys_imp::get_tid();

    if (_tid.load(sync::Ordering::Acquire) != tid) {
      return false;
    }

    if (_cnt.fetch_sub(1, sync::Ordering::AcqRel) == 1) {
      _tid.store(tid_t{}, sync::Ordering::Release);
      sys_imp::mtx_unlock(_raw);
    }
    return true;
  }
};

Mutex::Mutex() noexcept : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() noexcept -> Guard {
  _inn->lock();
  return Guard{&*_inn};
}

auto Mutex::try_lock() noexcept -> Option<Guard> {
  if (!_inn->try_lock()) {
    return {};
  }
  return {option::some_t{}, &*_inn};
}

Mutex::Guard::Guard(Inn* mtx) noexcept : _inn{mtx} {}

Mutex::Guard::~Guard() noexcept {
  if (_inn) {
    _inn->unlock();
  }
}

ReentrantLock::ReentrantLock() noexcept : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() noexcept -> Guard {
  _inn->lock();
  return Guard{&*_inn};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  if (!_inn->try_lock()) {
    return {};
  }
  return {option::some_t{}, &*_inn};
}

ReentrantLock::Guard::Guard(Inn* mtx) noexcept : _inn{mtx} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (!_inn) {
    return;
  }
  (void)_inn->unlock();
}

}  // namespace sfc::sync
