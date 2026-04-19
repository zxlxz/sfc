#pragma once

#include "sfc/sync/mutex.h"
#include "sfc/sync/condvar.h"
#include "sfc/collections/queue.h"

namespace sfc::sync::mpmc {

template <class T>
class Channel {
  Mutex _mtx{};
  Condvar _cnd{};
  collections::Queue<T> _msgs{};

 public:
  void send(T val) noexcept {
    auto lock = _mtx.lock();
    _msgs.push(static_cast<T&&>(val));
    _cnd.notify_one();
  }

  auto recv() noexcept -> T {
    auto lock = _mtx.lock();
    while (_msgs.is_empty()) {
      _cnd.wait(lock);
    }
    return _msgs.pop().unwrap();
  }

  auto recv_timeout(time::Duration dur) noexcept -> Option<T> {
    auto lock = _mtx.lock();
    if (_msgs.is_empty()) {
      _cnd.wait_timeout(lock, dur);
    }
    if (_msgs.is_empty()) {
      return {};
    }
    return _msgs.pop();
  }

  auto try_recv() noexcept -> Option<T> {
    auto lock = _mtx.lock();
    if (_msgs.is_empty()) {
      return {};
    }
    return _msgs.pop();
  }
};

template <class T>
struct Sender {
  Channel<T>& _chan;

 public:
  void send(T val) noexcept {
    _chan.send(static_cast<T&&>(val));
  }
};

template <class T>
struct Receiver {
  Channel<T>& _chan;

 public:
  auto recv() noexcept -> T {
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
