#pragma once

#include "sfc/thread.h"
#include "sfc/sync.h"
#include "sfc/collections/queue.h"

namespace sfc::task {

using Task = Box<void()>;

class Worker {
  using Task = Box<void()>;
  using TaskList = collections::Queue<Task>;

  static constexpr auto CAPACITY = 256U;

  sync::Mutex _mutex{};
  sync::Condvar _condvar{};
  sync::Atomic<bool> _running{true};
  TaskList _tasks;

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
