#pragma once

#include "sfc/alloc.h"

namespace sfc::sync {

class Mutex {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  struct Guard {
    Inn* _inn;

   public:
    explicit Guard(Inn* mtx) noexcept;
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
  };

  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;

  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

 public:
  struct Guard {
    Inn* _inn;

   public:
    explicit Guard(Inn* mtx) noexcept;
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
  };

  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

}  // namespace sfc::sync

namespace sfc::option {

template <>
struct Inner<sync::Mutex::Guard> {
  sync::Mutex::Guard _val;

 public:
  Inner(none_t) noexcept : _val{nullptr} {}
  Inner(some_t, auto&&... args) noexcept : _val{static_cast<decltype(args)&&>(args)...} {}
  ~Inner() noexcept = default;

  Inner(const Inner&) = delete;
  Inner& operator=(const Inner&) = delete;

  explicit operator bool() const noexcept {
    return _val._inn != nullptr;
  }
};
}  // namespace sfc::option
