#include "sfc/sync/reentrant_lock.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

struct ReentrantLock::Inn {
  sys_imp::Mutex _mutex{};
  u64            _owner{0};
  u32            _count{0};

 public:
  Inn()  = default;
  ~Inn() = default;

  void lock() {
    const auto thrd = sys_imp::Thread::current().raw();

    if (thrd != _owner) {
      _mutex.lock();
      __atomic_store_n(&_owner, thrd, __ATOMIC_SEQ_CST);
      __atomic_store_n(&_count, 1, __ATOMIC_SEQ_CST);
    } else {
      __atomic_fetch_add(&_count, 1, __ATOMIC_SEQ_CST);
    }
  }

  void unlock() {
    const auto thrd = sys_imp::Thread::current().raw();

    // not owner
    if (thrd != _owner) {
      return;
    }

    if (__atomic_fetch_sub(&_count, 1, __ATOMIC_SEQ_CST) == 1) {
      __atomic_store_n(&_owner, 0, __ATOMIC_SEQ_CST);
      _mutex.unlock();
    }
  }
};

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() = default;

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() -> ReentrantLockGuard {
  _inn ? _inn->lock() : (void)0;
  return ReentrantLockGuard{*this};
}

ReentrantLockGuard::ReentrantLockGuard(ReentrantLock& mtx) : _mtx{&mtx} {}

ReentrantLockGuard::~ReentrantLockGuard() noexcept {
  if (_mtx && _mtx->_inn) {
    _mtx->_inn->unlock();
  }
}

}  // namespace sfc::sync
