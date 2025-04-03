#pragma once

#include "sfc/thread.h"
#include "task.h"

namespace sfc::cyber {

class Sched {
  sync::Atomic<bool> _running{false};
  TaskQueue _task_queue;
  thread::JoinHandle _thread{};

 public:
  Sched();
  ~Sched();
  Sched(const Sched&) = delete;

  static auto global() -> Sched&;

  void start();

  void shutdown();

  void submit(Task task, Priority priority = Priority::Normal);

  void remove_task(Task task);

  void wait(const time::Duration& dur);
};

}  // namespace sfc::cyber
