#pragma once

#include "sfc/collections/queue.h"
#include "sfc/sync.h"

namespace sfc::cyber {

struct Task {
  void* _self = nullptr;
  void (*_func)(void*) = nullptr;

 public:
  explicit operator bool() const {
    return _func != nullptr;
  }
};

enum class Priority : u8 {
  RealTime = 0,
  High = 1,
  Normal = 2,
  Low = 3,
};

class TaskQueue {
 public:
  explicit TaskQueue();

  ~TaskQueue();

  TaskQueue(TaskQueue&&) noexcept;

  auto len() const -> usize;

  void push(Task task, Priority priority);

  auto try_pop() -> Option<Task>;

  auto pop_timeout(const time::Duration& dur) -> Option<Task>;

  void remove(Task task);

  void notify();

  void wait(const time::Duration& dur);

 private:
  Vec<Queue<Task>> _queues;
  sync::Atomic<usize> _count{0};

  sync::Mutex _mutex{};
  sync::Condvar _cv_push{};
  sync::Condvar _cv_pop{};

  void push_imp(Task task, Priority priority);
  auto pop_imp() -> Option<Task>;
};

}  // namespace sfc::cyber
