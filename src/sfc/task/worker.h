#pragma once

#include "sfc/thread.h"
#include "sfc/sync.h"
#include "sfc/collections.h"

namespace sfc::task {

using Task = Box<void()>;

class Worker {
  static constexpr auto CAPACITY = 256U;
  using Task = Box<void()>;

  sync::Mutex _mutex{};
  sync::Condvar _condvar{};
  sync::Atomic<bool> _running{true};
  VecDeque<Task> _task_queue;

 public:
  explicit Worker();
  ~Worker() noexcept;

  Worker(Worker&&) noexcept;
  Worker& operator=(Worker&&) noexcept;

  void run();
  void wait();
  void stop();
  auto post(Task task) -> bool;
};

}  // namespace sfc::task
