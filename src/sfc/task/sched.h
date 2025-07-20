#pragma once

#include "sfc/task/task.h"
#include "sfc/thread.h"

namespace sfc::task {

class Sched {
  TaskQueue _task_queue{};
  sync::Atomic<bool> _running{false};
  thread::JoinHandle _thread{{}};

 public:
  Sched();
  ~Sched();
  Sched(const Sched&) = delete;

  static auto global() -> Sched&;

  void start();

  void submit(Task task, Priority priority = Priority::Normal);

  void remove_task(Task task);

  void wait(const time::Duration& dur);
};

}  // namespace sfc::task
