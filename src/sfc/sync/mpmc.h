#pragma once

#include "sfc/sync/atomic.h"
#include "sfc/sync/ringbuf.h"

namespace sfc::sync::mpmc {

template <class T>
class Channel {
  using RingBuf = ringbuf::RingBuf<T>;
  RingBuf _buff{};
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
    res._buff = RingBuf::with_capacity(capacity);
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

  auto send(T val) noexcept -> bool {
    while (true) {
      if (this->is_closed()) {
        break;
      }
      if (_buff.push(static_cast<T&&>(val))) {
        return true;
      }
      sfc::thread::yield_now();
    }
    return false;
  }

  auto recv() noexcept -> Option<T> {
    while (true) {
      if (auto ret = _buff.pop()) {
        return ret;
      }
      if (this->is_closed()) {
        break;
      }
      sfc::thread::yield_now();
    }
    return {};
  }

  auto try_recv() noexcept -> Option<T> {
    return _buff.pop();
  }
};

template <class T>
struct Sender {
  Channel<T>& _chan;

 public:
  auto send(T val) noexcept -> bool {
    return _chan.send(static_cast<T&&>(val));
  }
};

template <class T>
struct Receiver {
  Channel<T>& _chan;

 public:
  auto recv() noexcept -> Option<T> {
    return _chan.recv();
  }

  auto recv_timeout(time::Duration dur) noexcept -> Option<T> {
    return _chan.recv_timeout(dur);
  }

  auto try_recv() noexcept -> Option<T> {
    return _chan.try_recv();
  }
};

}  // namespace sfc::sync::mpmc
