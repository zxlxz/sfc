#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread::sys {

struct Thread {
  u64 _id = static_cast<u64>(-1);

 public:
  static auto create(Box<void()> fun) -> Thread;

  auto id() const -> u64;
  void join();
};

void sleep(time::Duration dur);

}  // namespace sfc::thread::sys
