#pragma once

#include "sfc/core.h"
#include "sfc/sys/sync.h"

namespace sfc::sync {

class Mutex {
  struct Inn : sys::Mutex {};
  Inn _inn;

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
  friend class Mutex;
  Inn* _inn{nullptr};

 public:
  Guard(Inn& mtx) noexcept;
  ~Guard() noexcept;
  Guard(Guard&&) noexcept;
  Guard& operator=(Guard&&) noexcept;

 public:
  auto inner() -> sys::Mutex&;
};

class ReentrantLock {
  struct Inn : sys::Mutex {
    Atomic<u32> _owner{0};
    u32 _count{0};
  };
  Inn _inn;

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
  friend class ReentrantLock;
  Inn* _inn{nullptr};

 public:
  Guard(Inn& inn) noexcept;
  ~Guard() noexcept;
  Guard(Guard&&) noexcept;
  Guard& operator=(Guard&&) noexcept;
};

}  // namespace sfc::sync
