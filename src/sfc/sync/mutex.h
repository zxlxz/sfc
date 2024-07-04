#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::sync {

class Mutex {
  friend class LockGuard;
  friend class Condvar;

  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Mutex();
  ~Mutex();
  Mutex(Mutex&&) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

  [[nodiscard]] auto lock() -> class LockGuard;

 private:
  void unlock();
};

class LockGuard {
  friend class Mutex;
  friend class Condvar;
  ptr::Unique<Mutex> _mtx = {};

 public:
  ~LockGuard() {
    if (!_mtx) return;
    _mtx._ptr->unlock();
  }

  LockGuard(LockGuard&&) noexcept = default;

 private:
  explicit LockGuard(Mutex& mtx) noexcept : _mtx{&mtx} {}
};

}  // namespace sfc::sync
