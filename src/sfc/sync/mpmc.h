#pragma once

#include "sfc/sync/ringbuf.h"

namespace sfc::sync::mpmc {

template <class T>
class Channel {
  RingBuf<T> _ringbuf{};
  Atomic<bool> _closed{true};

 public:
  Channel() noexcept {}

  ~Channel() noexcept {
    this->close();
  }

  Channel(Channel&&) noexcept = default;
  Channel& operator=(Channel&&) noexcept = default;

  static auto with_capacity(usize capacity) -> Channel {
    if (capacity == 0) {
      return {};
    }

    auto res = Channel{};
    res._ringbuf = RingBuf<T>::with_capacity(capacity);
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
    while (true) {
      if (this->is_closed()) {
        break;
      }
      if (_ringbuf.try_push(val)) {
        return {};
      }
      thread::yield_now();
    }
    return mem::move(val);
  }

  auto recv() noexcept -> Option<T> {
    while (true) {
      if (auto opt = _ringbuf.pop()) {
        return opt;
      }
      if (this->is_closed()) {
        break;
      }
      thread::yield_now();
    }
    return {};
  }

  auto try_send(T val) noexcept -> Option<T> {
    if (!this->is_closed()) {
      if (_ringbuf.try_push(val)) {
        return {};
      }
    }
    return mem::move(val);
  }

  auto try_recv() noexcept -> Option<T> {
    return _ringbuf.pop();
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
