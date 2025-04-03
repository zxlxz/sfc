#pragma once

#include "sfc/collections.h"
#include "sfc/sync.h"

namespace sfc::cyber {

template <class T>
class MsgQueue {
  sync::Mutex _mutex{};
  sync::Condvar _condvar{};

  CircBuf<T> _buff;

 public:
  explicit MsgQueue(usize capacity) : _buff{capacity} {}

  ~MsgQueue() {}

  MsgQueue(const MsgQueue&) = delete;

  auto pop() -> Option<T> {
    auto lock = _mutex.lock();
    if (_buff.is_empty()) {
      return {};
    }

    auto res = _buff.pop_front();
    return res;
  }

  auto push(T msg, const time::Duration& try_dur) -> bool {
    auto lock = _mutex.lock();
    if (_buff.is_full()) {
      _condvar.wait_timeout(lock, try_dur);
    }

    const auto res = !_buff.is_full();
    _buff.push_back(static_cast<T&&>(msg));
    return res;
  }
};

}  // namespace sfc::cyber
