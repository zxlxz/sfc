#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"
#include "sfc/sys.h"

namespace sfc::thread {

struct Thread {
  u32 id;
};

class JoinHandle {
  friend struct Builder;
  sys::Thread _thread{};

 public:
  JoinHandle() noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&& other) noexcept;
  JoinHandle& operator=(JoinHandle&& other) noexcept;
};

struct Builder {
  usize stack_size = 0;
  Str name = {};

 public:
  auto spawn(Box<void()> f) -> JoinHandle;
};

auto current() -> Thread;
auto current_id() -> u32;

void yield_now();

void sleep(time::Duration dur);
void sleep_ms(u32 ms);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>{mem::move(f)});
}

}  // namespace sfc::thread
