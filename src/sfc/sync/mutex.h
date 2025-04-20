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
};

class [[nodiscard]] LockGuard {
  friend class Condvar;
  ptr::Unique<Mutex> _mtx = {};

 public:
  explicit LockGuard(Mutex& mtx);
  ~LockGuard();
};

}  // namespace sfc::sync
