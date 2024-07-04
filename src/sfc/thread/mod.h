#pragma once

#include "sys/thread.h"

namespace sfc::thread {

struct Thread {
  static constexpr u64 INVALID_ID = static_cast<u64>(-1);

  u64 _id = INVALID_ID;
};

class JoinHandle {
  Thread _thr;

 public:
  explicit JoinHandle(Thread thr) noexcept;
  JoinHandle();
  JoinHandle(JoinHandle&&) noexcept;
  ~JoinHandle();

  JoinHandle& operator=(JoinHandle&&) noexcept;

  void join();
};

struct Builder {
  auto spawn(Box<void()> f) -> JoinHandle;
};

void sleep(time::Duration dur);
void sleep_ms(u32 ms);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>{mem::move(f)});
}

}  // namespace sfc::thread
