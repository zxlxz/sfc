#pragma once

#include "sfc/sync/atomic.h"
#include "sfc/sync/mutex.h"
#include "sfc/sync/condvar.h"
#include "sfc/collections/queue.h"

namespace sfc::sync::mpmc {

template <class T>
class Channel {
  static constexpr auto DEFAULT_CAPACITY = usize{16};
  using MsgQueue = collections::Queue<T>;

  Mutex _mtx{};
  Condvar _send_evt{};
  Condvar _recv_evt{};
  Atomic<bool> _closed{false};
  MsgQueue _msgs{};

 public:
  Channel() noexcept {}

  ~Channel() noexcept {
    this->close();
  }

  Channel(Channel&&) noexcept = default;
  Channel& operator=(Channel&&) noexcept = default;

  static auto with_capacity(usize capacity) -> Channel {
    auto res = Channel{};
    res._msgs.reserve(capacity);
    return res;
  }

 public:
  void close() {
    if (_closed.exchange(true, Ordering::AcqRel)) {
      return;
    }
    _send_evt.notify_all();
    _recv_evt.notify_all();
  }

  auto is_closed() const noexcept -> bool {
    return _closed.load(Ordering::Acquire);
  }

  auto send(T val) noexcept -> bool {
    if (this->is_closed()) {
      return false;
    }
    auto lock = _mtx.lock();
    if (_msgs.capacity() == 0) {
      _msgs.reserve(DEFAULT_CAPACITY);
    }
    _send_evt.wait_while(lock, [&]() { return _msgs.is_full() && !this->is_closed(); });
    return this->push_imp(static_cast<T&&>(val));
  }

  auto recv() noexcept -> Option<T> {
    auto lock = _mtx.lock();
    _recv_evt.wait_while(lock, [&]() { return _msgs.is_empty() && !this->is_closed(); });
    return this->pop_imp();
  }

  auto recv_timeout(time::Duration dur) noexcept -> Option<T> {
    auto lock = _mtx.lock();
    if (_msgs.is_empty() && !this->is_closed()) {
      _recv_evt.wait_timeout(lock, dur);
    }
    return this->pop_imp();
  }

  auto try_recv() noexcept -> Option<T> {
    auto lock = _mtx.lock();
    return this->pop_imp();
  }

 private:
  auto push_imp(T val) noexcept -> bool {
    if (_msgs.is_full()) {
      return false;
    }
    _msgs.push(static_cast<T&&>(val));
    _recv_evt.notify_one();
    return true;
  }

  auto pop_imp() noexcept -> Option<T> {
    auto res = _msgs.pop();
    if (res.is_some()) {
      _send_evt.notify_one();
    }
    return res;
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
