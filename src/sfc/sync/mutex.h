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

class Mutex::Guard {
  ptr::Unique<Mutex> _lock;

 public:
  Guard(Mutex&, passkey_t = {});
  ~Guard() noexcept;
  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;

  auto inner() -> sys::Mutex&;
};

class ReentrantLock {
  struct passkey_t {};
  struct Tid {
    Atomic<u32> _id;
    auto contains(u32 tid) const -> bool;
    void set(u32 tid);
  };

  sys::Mutex _mutex;
  Tid _owner;
  u32 _count;

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

class ReentrantLock::Guard {
  ptr::Unique<ReentrantLock> _lock;

 public:
  Guard(ReentrantLock&, passkey_t = {});
  ~Guard() noexcept;
  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;
};

}  // namespace sfc::sync
