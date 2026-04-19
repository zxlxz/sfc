#pragma once

#include "sfc/sys.h"
#include "sfc/sync/atomic.h"

namespace sfc::sync {

class Mutex {
  struct passkey_t {};
  sys::Mutex _inn;

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  class Guard;
  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct passkey_t {};
  sys::Mutex _mutex;
  Atomic<u32> _owner;
  Atomic<u32> _count;

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;

  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

 public:
  class Guard;
  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class Mutex::Guard {
  Mutex& _lock;

 public:
  Guard(Mutex&, passkey_t = {});
  ~Guard() noexcept;

  Guard(const Guard&) = delete;
  Guard& operator=(const Guard&) = delete;

  auto inner() -> sys::Mutex&;
};

class ReentrantLock::Guard {
  ReentrantLock& _lock;

 public:
  Guard(ReentrantLock&, passkey_t = {});
  ~Guard() noexcept;
  Guard(const Guard&) = delete;
  void operator=(const Guard&) = delete;
};

}  // namespace sfc::sync
