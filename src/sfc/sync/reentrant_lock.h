#pragma once

#include "sfc/sync/mutex.h"

namespace sfc::sync {

class ReentrantLock {
  friend class ReentrantLockGuard;

  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock();
  ~ReentrantLock();
  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

  [[nodiscard]] auto lock() -> class ReentrantLockGuard;
};

class [[nodiscard]] ReentrantLockGuard {
  ptr::Unique<ReentrantLock> _mtx = {};

 public:
  explicit ReentrantLockGuard(ReentrantLock& mtx);
  ~ReentrantLockGuard() noexcept;
};

}  // namespace sfc::sync
