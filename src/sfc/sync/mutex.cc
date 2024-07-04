#include "mutex.h"

#include <pthread.h>

namespace sfc::sync {

struct Mutex::Inn {
  pthread_mutex_t _raw = PTHREAD_MUTEX_INITIALIZER;

  void lock() {
    ::pthread_mutex_lock(&_raw);
  }

  void unlock() {
    ::pthread_mutex_unlock(&_raw);
  }
};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> LockGuard {
  if (_inn) {
    _inn.ptr()->lock();
  }
  return LockGuard{*this};
}

void Mutex::unlock() {
  if (!_inn) {
    return;
  }
  _inn.ptr()->unlock();
}

}  // namespace sfc::sync
