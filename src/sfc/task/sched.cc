#include "sfc/task/sched.h"

#include "sfc/sync.h"
#include "sfc/thread.h"

namespace sfc::task {

Sched::Sched() {
  this->start();
}

Sched::~Sched() {
  _running.exchange(false);
  _task_queue.notify();
}

auto Sched::global() -> Sched& {
  static Sched res{};
  return res;
}

void Sched::start() {
  if (_running.exchange(true)) {
    return;
  }

  _thread = thread::spawn([&]() {
    while (_running.load()) {
      auto task_opt = _task_queue.pop_timeout(time::Duration::from_millis(100U));
      if (!task_opt) {
        continue;
      }

      try {
        auto task = task_opt.unwrap();
        (task._func)(task._self);
      } catch (...) {}
    }
  });
}

void Sched::submit(Task task, Priority priority) {
  if (!task || !_running.load()) {
    return;
  }
  _task_queue.push(task, priority);
}

void Sched::remove_task(Task task) {
  _task_queue.remove(task);
}

void Sched::wait(const time::Duration& dur) {
  if (!_running.load()) {
    return;
  }
  _task_queue.wait(dur);
}

}  // namespace sfc::task
