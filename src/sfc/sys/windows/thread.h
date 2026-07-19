#pragma once

#include "sfc/alloc.h"

namespace sfc::sys::windows {

struct Thread {
  void* _handle{nullptr};

  Thread() noexcept = default;
  explicit Thread(void* handle) noexcept;

  static auto spawn(usize stack_size, Box<void()> f) -> Thread;

  auto is_valid() const -> bool;
  void join();
  void detach();
};

struct ThisThread {
  static auto id() noexcept -> u32;
  static auto set_name(const char* name) -> bool;

  static void yield();
  static void sleep(time::Duration dur) noexcept;
};

}  // namespace sfc::sys::windows
