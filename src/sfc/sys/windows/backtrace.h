#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

struct StackFrame {
  const char* file = nullptr;
  char func[256] = {};

 public:
  static auto from_addr(const void* addr) -> StackFrame;
};

struct Backtrace {
  constexpr static auto kMaxFrame = 64U;
  void* _frames[kMaxFrame];
  usize _len = 0;

 public:
  static auto capture() -> Backtrace;

  auto len() const -> usize;
  auto frame(usize idx) const -> StackFrame;
};

}  // namespace sfc::sys::windows
