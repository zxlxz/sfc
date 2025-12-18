#pragma once

#include "sfc/sync/mutex.h"
#include "sfc/sync/condvar.h"
#include "sfc/time/duration.h"
#include "sfc/collections/vec_deque.h"

namespace sfc::sync {

template <class T>
struct Sender;

template <class T>
struct Receiver;

template <class T>
class Channel {
  Mutex _mtx{};
  Condvar _cnd{};
  collections::VecDeque<T> _queue{};

 public:
  friend struct Sender<T>;
  auto sender() noexcept -> Sender<T> {
    return Sender<T>{*this};
  }

  friend struct Receiver<T>;
  auto receiver() noexcept -> Receiver<T> {
    return Receiver<T>{*this};
  }

 private:
  auto lock() noexcept -> Mutex::Guard {
    return _mtx.lock();
  }
};

template <class T>
struct Sender {
  Channel<T>& _chan;

 public:
  void send(T val) noexcept {
    auto lock = _chan._mtx.lock();
    _chan._queue.push_back(static_cast<T&&>(val));
    _chan._cnd.notify_one();
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
    auto lock = _chan.lock();
  }
};

}  // namespace sfc::sync
