#pragma once

#include "sfc/sync/mutex.h"

namespace sfc::sync {

class ReentrantLock {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock();
  ~ReentrantLock();

  ReentrantLock(ReentrantLock&&) noexcept = default;
  ReentrantLock& operator=(ReentrantLock&&) noexcept = default;

  class Guard;
  [[nodiscard]] auto lock() -> Guard;
};

class [[nodiscard]] ReentrantLock::Guard {
  ptr::Unique<Inn> _inn = {};

 public:
  explicit Guard(Inn& mtx);
  ~Guard() noexcept;

  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;
};

}  // namespace sfc::sync
