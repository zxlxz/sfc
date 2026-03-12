#pragma once

#include "sfc/sys.h"
#include "sfc/sync/atomic.h"

namespace sfc::sync {

class Mutex {
  sys::Mutex _inn;

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  class Guard {
    Mutex& _lock;

   public:
    Guard(trait::passkey_t<Mutex>);
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;

    auto inner() -> sys::Mutex&;
  };
  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct key_t;
  sys::Mutex _mutex;
  Atomic<u32> _owner;
  Atomic<u32> _count;

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;

  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

 public:
  class Guard {
    ReentrantLock& _lock;

   public:
    Guard(trait::passkey_t<ReentrantLock>);
    ~Guard() noexcept;
    Guard(const Guard&) = delete;
    void operator=(const Guard&) = delete;
  };
  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

}  // namespace sfc::sync
