#pragma once

#include "sfc/sync/ringbuf.h"

namespace sfc::sync::mpmc {

template <class T>
class Channel {
  using Queue = RingBuf<T>;
  Queue _queue{};
  Atomic<bool> _closed{true};

 public:
  Channel() noexcept {}

  ~Channel() noexcept {
    this->close();
  }

  Channel(Channel&&) noexcept = default;
  Channel& operator=(Channel&&) noexcept = default;

  static auto with_capacity(usize capacity) -> Channel {
    if (capacity == 0) return {};
    auto res = Channel{};
    res._queue = Queue::with_capacity(capacity);
    res._closed.store(false);
    return res;
  }

 public:
  void close() {
    _closed.exchange(true, Ordering::AcqRel);
  }

  auto is_closed() const noexcept -> bool {
    return _closed.load(Ordering::Acquire);
  }

  auto send(T val) noexcept -> Option<T> {
    const auto ret = this->try_send(val);
    if (ret) {
      return {};
    }
    return Option{mem::move(val)};
  }

  auto recv() noexcept -> Option<T> {
    while (!this->is_closed()) {
      if (auto opt = _queue.pop()) {
        return opt;
      }
      sfc::thread::yield_now();
    }
    return {};
  }

  auto try_send(T& val) noexcept -> bool {
    while (!this->is_closed()) {
      if (_queue.try_push(val)) {
        return true;
      }
      sfc::thread::yield_now();
    }
    return false;
  }

  auto try_recv() noexcept -> Option<T> {
    return _queue.pop();
  }
};

template <class T>
struct Sender {
  Channel<T>& _chan;

 public:
  auto send(T val) noexcept -> Option<T> {
    return _chan.send(mem::move(val));
  }
};

template <class T>
struct Receiver {
  Channel<T>& _chan;

 public:
  auto recv() noexcept -> Option<T> {
    return _chan.recv();
  }

  auto try_recv() noexcept -> Option<T> {
    return _chan.try_recv();
  }
};

}  // namespace sfc::sync::mpmc
