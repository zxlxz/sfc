#pragma once

#include "sfc/core.h"
#include "sfc/sys.h"

namespace sfc::sync {

class Mutex {
  using Inn = sys::Mutex;
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
  ptr::Unique<Mutex> _lock;

 private:
  Guard(Mutex&);

 public:
  ~Guard() noexcept;
  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;
  auto inner() -> Inn&;
};

class ReentrantLock {
  struct Inn {
    sys::Mutex _imp{};
    Atomic<u32> _owner{0};
    u32 _count{0};

   public:
    void lock();
    auto try_lock() -> bool;
    void unlock();
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
  ptr::Unique<ReentrantLock> _lock;

 private:
  Guard(ReentrantLock&);

 public:
  ~Guard() noexcept;
  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;
};

}  // namespace sfc::sync
