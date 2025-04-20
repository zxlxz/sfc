#include "reentrant_lock.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

struct ReentrantLock::Inn {
  sys_imp::Mutex  _mutex{};
  sys_imp::Thread _owner{};
  u32             _count{0};

 public:
  Inn() = default;
  ~Inn() = default;

  void lock() {
    const auto thrd = sys_imp::Thread::current();

    if (thrd == _owner) {
      __atomic_fetch_add(&_count, 1, __ATOMIC_SEQ_CST);
    } else {
      _mutex.lock();
      _owner = thrd;
      __atomic_fetch_add(&_count, 1, __ATOMIC_SEQ_CST);
    }
  }

  void unlock() {
    const auto thrd = sys_imp::Thread::current();

    // not owner
    if (thrd != _owner) {
      return;
    }

    if (__atomic_fetch_sub(&_count, 1, __ATOMIC_SEQ_CST) == 1) {
      _owner = sys_imp::Thread{};
      _mutex.unlock();
    }
  }
};

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() -> ReentrantLockGuard {
  _inn ? _inn->lock() : (void)0;
  return ReentrantLockGuard{*this};
}

ReentrantLockGuard::ReentrantLockGuard(ReentrantLock& mtx) : _mtx{&mtx} {}

ReentrantLockGuard::~ReentrantLockGuard() noexcept {
  if (!_mtx || !_mtx->_inn) {
    return;
  }
  _mtx->_inn->unlock();
}

}  // namespace sfc::sync
