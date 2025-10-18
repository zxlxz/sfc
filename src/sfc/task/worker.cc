#include "sfc/task.h"

#include "sfc/collections/queue.h"
#include "sfc/sync.h"
#include "sfc/thread.h"

namespace sfc::task {

using sync::Ordering;

Worker::Worker() : _task_queue{VecDeque<Task>::with_capacity(CAPACITY)} {}

Worker::~Worker() noexcept {
  this->stop();
}

Worker::Worker(Worker&&) noexcept = default;

Worker& Worker::operator=(Worker&&) noexcept = default;

void Worker::run() {
  while (_running.load<Ordering::Acquire>()) {
    auto lock = _mutex.lock();
    _condvar.wait_while(lock, [&]() { return _task_queue.is_empty() && _running.load(); });

    if (!_running.load<Ordering::Acquire>() || _task_queue.is_empty()) {
      break;
    }

    auto task = _task_queue.pop_front().unwrap();
    try {
      task();
    } catch (...) {
      break;
    }
  }
}

void Worker::stop() {
  _running.store<Ordering::Release>(false);
  _condvar.notify_all();
}

void Worker::wait() {
  while (true) {
    if (!_running.load<Ordering::Acquire>()) {
      break;
    }

    auto lock = _mutex.lock();
    if (_task_queue.is_empty()) {
      break;
    }
    thread::yield_now();
  }
}

auto Worker::post(Task task) -> bool {
  if (!_running.load<Ordering::Acquire>()) {
    return false;
  }

  auto lock = _mutex.lock();
  if (_task_queue.len() >= CAPACITY) {
    return false;
  }
  _task_queue.push_back(static_cast<Task&&>(task));
  _condvar.notify_one();
  return true;
}

}  // namespace sfc::task
