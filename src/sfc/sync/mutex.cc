#include "mutex.h"

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
};

struct ReentrantLock::Inn {
  mtx_t _raw = {};
  tid_t _tid = {};
  i32 _cnt = {0};

 public:
  Inn() noexcept {
    sys_imp::mtx_init(_raw);
  }

  ~Inn() noexcept {
    sys_imp::mtx_destroy(_raw);
  }

  void lock() noexcept {
    const auto tid = sys_imp::get_tid();

    if (tid == __atomic_load_n(&_tid, 0)) {
      __atomic_fetch_add(&_cnt, 1, 0);
      return;
    }

    sys_imp::mtx_lock(_raw);
    __atomic_store_n(&_tid, tid, 0);
    __atomic_store_n(&_cnt, 1, 0);
  }

  auto unlock() noexcept -> bool {
    const auto tid = sys_imp::get_tid();

    if (tid != __atomic_load_n(&_tid, 0)) {
      return false;
    }

    if (__atomic_fetch_sub(&_cnt, 1, 0) == 1) {
      __atomic_store_n(&_tid, tid_t{}, 0);
      sys_imp::mtx_unlock(_raw);
    }
    return true;
  }
};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> Guard {
  _inn->lock();
  return Guard{*_inn};
}

Mutex::Guard::Guard(Inn& mtx) noexcept : _inn{mtx} {}

Mutex::Guard::~Guard() noexcept {
  _inn.unlock();
}

ReentrantLock::ReentrantLock() noexcept : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() -> Guard {
  _inn->lock();
  return Guard{*_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) noexcept : _inn{inn} {}

ReentrantLock::Guard::~Guard() noexcept {
  _inn.unlock();
}

}  // namespace sfc::sync
