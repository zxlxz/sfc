#include "sfc/task/worker.h"

namespace sfc::task {

using sync::Ordering;

Worker::Worker() : _tasks{TaskList::with_capacity(CAPACITY)} {}

Worker::~Worker() noexcept {
  this->stop();
}

Worker::Worker(Worker&&) noexcept = default;

Worker& Worker::operator=(Worker&&) noexcept = default;

void Worker::run() {
  while (_running.load(sync::Ordering::Acquire)) {
    auto lock = _mutex.lock();
    _condvar.wait_while(lock, [&]() { return _tasks.is_empty() && _running.load(); });

    if (!_running.load(sync::Ordering::Acquire) || _tasks.is_empty()) {
      break;
    }

    auto task = _tasks.pop().unwrap();
    try {
      task();
    } catch (...) {
      break;
    }
  }
}

void Worker::stop() {
  _running.store(false, sync::Ordering::Release);
  _condvar.notify_all();
}

void Worker::wait() {
  while (true) {
    if (!_running.load(sync::Ordering::Acquire)) {
      break;
    }

    auto lock = _mutex.lock();
    if (_tasks.is_empty()) {
      break;
    }
    thread::yield_now();
  }
}

auto Worker::post(Task task) -> bool {
  if (!_running.load(sync::Ordering::Acquire)) {
    return false;
  }

  auto lock = _mutex.lock();
  if (_tasks.len() >= CAPACITY) {
    return false;
  }
  _tasks.push(static_cast<Task&&>(task));
  _condvar.notify_one();
  return true;
}

}  // namespace sfc::task
