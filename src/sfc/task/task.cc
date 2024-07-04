#include "sfc/task.h"

#include "sfc/collections/queue.h"
#include "sfc/sync.h"
#include "sfc/thread.h"

namespace sfc::task {

using collections::VecDeque;
using sync::Ordering;

class Worker::Inn {
  static constexpr auto kMaxTaskCnt = 256U;

  sync::Mutex _mutex{};
  sync::Condvar _condvar{};
  sync::Atomic<bool> _running{true};
  sync::Atomic<usize> _inflight_cnt{0};

  VecDeque<Task> _task_queue;
  thread::JoinHandle _thread;

 public:
  Inn()
      : _task_queue{_task_queue.with_capacity(256)}
      , _thread{thread::spawn([this] { this->run(); })} {}

  ~Inn() {
    this->stop();
  }

  auto is_running() const -> bool {
    return _running.load<Ordering::Relaxed>();
  }

  auto is_full() const -> bool {
    return _inflight_cnt.load<Ordering::Acquire>() >= kMaxTaskCnt;
  }

  void stop() {
    _running.store<Ordering::Release>(false);
    _condvar.notify_all();
  }

  auto push(Task&& task) -> bool {
    if (this->is_full() || !this->is_running()) {
      return false;
    }

    auto lock = _mutex.lock();
    _task_queue.push_back(static_cast<Task&&>(task));
    _inflight_cnt.fetch_add<Ordering::Release>(1);
    _condvar.notify_one();
    return true;
  }

  auto pop() -> Task {
    auto lock = _mutex.lock();
    _condvar.wait_while(lock, [&]() { return _task_queue.is_empty() && _running.load(); });

    if (!_running.load<Ordering::Acquire>() || _task_queue.is_empty()) {
      return {};
    }
    return _task_queue.pop_front().unwrap_or({});
  }

  void run() {
    while (_running.load<Ordering::Acquire>()) {
      auto task = this->pop();
      try {
        task ? task() : void();
      } catch (...) {}
      _inflight_cnt.fetch_sub<Ordering::Release>(1);
    }

    auto lock = _mutex.lock();
    _task_queue.clear();
    _inflight_cnt.store<Ordering::Release>(0);
  }

  void wait() {
    while (_inflight_cnt.load<Ordering::Acquire>() != 0) {
      thread::yield_now();
    }
  }
};

Worker::Worker() : _inn{Box<Inn>::xnew()} {}

Worker::~Worker() {}

Worker::Worker(Worker&&) noexcept = default;

Worker& Worker::operator=(Worker&&) noexcept = default;

auto Worker::is_running() const -> bool {
  return _inn && _inn->is_running();
}

auto Worker::is_full() const -> bool {
  return _inn && _inn->is_full();
}

void Worker::stop() {
  if (!_inn) {
    return;
  }
  _inn->stop();
}

void Worker::wait() {
  if (!_inn) {
    return;
  }
  _inn->wait();
}

auto Worker::post(Task task) -> bool {
  if (!_inn) {
    return false;
  }
  return _inn->push(mem::move(task));
}

}  // namespace sfc::task
