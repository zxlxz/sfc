#pragma once

#include "sfc/sys.h"
#include "sfc/sync/atomic.h"

namespace sfc::sync {

class Condvar;

class Mutex {
  friend class Condvar;
  sys::Mutex _inn;

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  class Guard {
    friend class Condvar;
    Mutex& _lock;

   public:
    Guard(trait::passkey_t<Mutex>);
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
  };
  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct key_t;
#ifdef _SFC_SYS_SYNC_
  sys::Mutex _mutex;
#else
  void* _mutex;
#endif
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
