#pragma once

#include "sfc/msg/chan.h"

namespace sfc::msg {

template <class T>
class Send {
  Rc<Chan<T>> _chan;

 public:
  explicit Send(Rc<Chan<T>> chan) : _chan{mem::move(chan)} {}

  explicit Send(Str topic) : _chan{ChanManager<T>::instance().get(topic)} {}

  void send(T value) {
    _chan->push(static_cast<T&&>(value));
  }
};

}  // namespace sfc::cyber
