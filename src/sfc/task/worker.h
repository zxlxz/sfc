#pragma once

#include "sfc/thread.h"
#include "sfc/sync.h"
#include "sfc/collections/vec_deque.h"

namespace sfc::task {

using Task = Box<void()>;

class Worker {
  using Task = Box<void()>;
  using TaskQueue = collections::VecDeque<Task>;

  static constexpr auto CAPACITY = 256U;

  sync::Mutex _mutex{};
  sync::Condvar _condvar{};
  sync::Atomic<bool> _running{true};
  TaskQueue _task_queue;

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
