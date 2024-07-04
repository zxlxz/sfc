#pragma once

#include "sfc/alloc.h"

namespace sfc::task {

class Worker {
  using Task = Box<void()>;

  class Inn;
  Box<Inn> _inn{};

 public:
  Worker();
  ~Worker() noexcept;

  Worker(Worker&&) noexcept;
  Worker& operator=(Worker&&) noexcept;

  auto is_running() const -> bool;

  auto is_full() const -> bool;

  void stop();

  void wait();

  auto post(Task task) -> bool;

  auto submit(auto task) -> bool {
    if (!this->is_running() || this->is_full()) {
      return false;
    }
    auto box_task = Task::xnew(static_cast<decltype(task)&&>(task));
    return this->post(mem::move(box_task));
  }
};

}  // namespace sfc::task
