
#include "sfc/task/task.h"

#include "sfc/collections.h"

namespace sfc::task {

TaskQueue::TaskQueue() {
  static constexpr const auto QUEUE_CAPACITY = 64U;

  const auto que_cnt = static_cast<usize>(Priority::Low) + 1;
  for (auto i = 0U; i < que_cnt; ++i) {
    _queues.push(Queue<Task>::with_capacity(QUEUE_CAPACITY));
  }
}

TaskQueue::~TaskQueue() = default;

TaskQueue::TaskQueue(TaskQueue&&) noexcept = default;

auto TaskQueue::len() const -> usize {
  return _count.load();
}

void TaskQueue::push(Task task, Priority priority) {
  auto lock = _mutex.lock();
  this->push_imp(task, priority);
}

auto TaskQueue::try_pop() -> Option<Task> {
  if (_count.load() == 0) {
    return {};
  }

  auto lock = _mutex.lock();
  return this->pop_imp();
}

auto TaskQueue::pop_timeout(const time::Duration& dur) -> Option<Task> {
  auto lock = _mutex.lock();

  if (auto task = this->pop_imp()) {
    return task;
  }

  if (_cv_push.wait_timeout(lock, dur)) {
    return this->pop_imp();
  }

  return {};
}

void TaskQueue::remove(Task task) {
  auto lock = _mutex.lock();

  auto count = 0UL;
  for (auto& tasks : _queues.as_mut_slice()) {
    tasks.retain([&](auto& x) { return x._self != task._self; });
    count += tasks.len();
  }
  _count.store(count);
}

void TaskQueue::notify() {
  _cv_push.notify_all();
}

void TaskQueue::wait(const time::Duration& dur) {
  if (_count.load() < _queues.len()) {
    return;
  }
  auto lock = _mutex.lock();
  _cv_pop.wait_timeout(lock, dur);
}

void TaskQueue::push_imp(Task task, Priority priority) {
  const auto pid = cmp::min(static_cast<usize>(priority), _queues.len() - 1);

  // check valid
  auto& tasks = _queues[pid];
  tasks.push(task);
  _count.fetch_add(1);

  // notify one
  if (tasks.len() == 1) {
    _cv_push.notify_one();
  } else {
  }
}

auto TaskQueue::pop_imp() -> Option<Task> {
  if (_count.load() == 0) {
    return {};
  }

  // pop imp
  for (auto& q : _queues.as_mut_slice()) {
    if (auto task = q.pop()) {
      if (_count.fetch_sub(1) == 1) {
        _cv_pop.notify_one();
      }
      return task;
    }
  }
  return {};
}

}  // namespace sfc::task
