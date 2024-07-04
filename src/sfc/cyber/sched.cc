#include "sched.h"

#include "sfc/sync.h"
#include "sfc/thread.h"

namespace sfc::cyber {

Sched::Sched() {
  this->start();
}

Sched::~Sched() {
  this->shutdown();
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
    while (__builtin_expect(_running.load(), true)) {
      auto task_opt = _task_queue.pop_timeout_ms(100U);
      if (!task_opt) {
        continue;
      }

      try {
        auto task = task_opt.unwrap();
        (task._func)(task._self);
      } catch (const panicking::Error& e) {
        __builtin_printf("%s\n", e.what());
      }
    }
  });
}

void Sched::shutdown() {
  if (!_running.exchange(false)) {
    return;
  }
  _task_queue.notify();
  _thread.join();
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

void Sched::wait(u32 msec) {
  if (!_running.load()) {
    return;
  }
  _task_queue.wait(msec);
}

}  // namespace sfc::cyber
