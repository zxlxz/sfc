#pragma once

#include "sfc/sync/mutex.h"
#include "sfc/sync/condvar.h"
#include "sfc/time/duration.h"
#include "sfc/collections/vec_deque.h"

namespace sfc::sync::mpsc {

template <class T>
struct Sender;

template <class T>
struct Receiver;

template <class T>
class Channel {
  Mutex _mtx{};
  Condvar _cnd{};
  collections::VecDeque<T> _msg_queue{};

 public:
  void send(T val) noexcept {
    auto guard = _mtx.lock();
    _msg_queue.push_back(static_cast<T&&>(val));
    _cnd.notify_one();
  }

  auto try_recv() noexcept -> Option<T> {
    auto guard = _mtx.lock();
    return _msg_queue.pop_front();
  }

  auto recv() noexcept -> T {
    auto guard = _mtx.lock();
    while (true) {
      if (auto msg_opt = _msg_queue.pop_front(); msg_opt.is_some()) {
        return static_cast<T&&>(*msg_opt);
      }
      _cnd.wait(guard);
    }
  }

  auto recv_timeout(time::Duration dur) noexcept -> Option<T> {
    auto guard = _mtx.lock();
    while (true) {
      if (auto msg_opt = _msg_queue.pop_front(); msg_opt.is_some()) {
        return msg_opt;
      }
      if (!_cnd.wait_timeout(guard, dur)) {
        return {};
      }
    }
  }

 public:
  auto sender() noexcept -> Sender<T> {
    return Sender<T>{*this};
  }

  auto receiver() noexcept -> Receiver<T> {
    return Receiver<T>{*this};
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
  explicit Receiver(Channel<T>& chan) noexcept : _chan{chan} {}
  ~Receiver() noexcept = default;

  Receiver(const Receiver&) = delete;
  Receiver& operator=(const Receiver&) = delete;

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

}  // namespace sfc::sync::mpsc
