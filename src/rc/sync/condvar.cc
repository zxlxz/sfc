#include "rc.inl"

#include "rc/sync/condvar.h"

namespace rc::sync {

pub CondVar::CondVar() = default;
pub CondVar::~CondVar() = default;
pub CondVar::CondVar(CondVar&& other) noexcept = default;

pub auto CondVar::wait(MutexGuard& guard) -> void {
  _inner.wait(guard._mutex->_inner);
}

pub auto CondVar::wait_timeout(MutexGuard& guard, Duration dur) -> bool {
  return _inner.wait_timeout(guard._mutex->_inner, dur);
}

pub auto CondVar::notify_one() -> void { return _inner.notify_all(); }

pub auto CondVar::notify_all() -> void { return _inner.notify_all(); }

}  // namespace rc::sync
