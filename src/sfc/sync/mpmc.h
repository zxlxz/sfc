#pragma once

#include "sfc/collections/vec_deque.h"
#include "sfc/sync/mutex.h"

namespace sfc::sync {

template <class T>
class Chan;

template <class T>
class Sender {
  friend Chan<T>;
  Chan<T>* _chan{};

 public:
  explicit Sender(Chan<T>& chan) : _chan{&chan} {
    chan.add_sender(*this);
  }

  ~Sender() {
    _chan ? _chan->remove_sender(*this) : void();
  }

  Sender(const Sender& other) = delete;
  Sender& operator=(const Sender& other) = delete;

  auto send(T value) -> bool {
    if (!_chan) {
      return false;
    }
    _chan->push(static_cast<T&&>(value));
    return true;
  }
};

template <class T>
class Receiver {
  using MsgQue = collections::VecDeque<Rc<T>>;

  friend Chan<T>;

  Mutex _mutex{};
  Chan<T>* _chan{};
  MsgQue _msgq{};

 public:
  Receiver(Chan<T>& chan, usize capacity = 64) : _chan{&chan}, _msgq{MsgQue::with_capacity(capacity)} {
    chan.add_receiver(*this);
  }

  ~Receiver() {
    _chan ? _chan->remove_receiver(*this) : void();
  }

  Receiver(const Receiver& other) = delete;
  Receiver& operator=(const Receiver& other) = delete;

  void notify(Rc<T> msg) {
    auto lock = _mutex.lock();
    if (_msgq.len() >= _msgq.capacity()) {
      _msgq.pop_front();
    }

    _msgq.push_back(static_cast<Rc<T>&&>(msg));
  }

  auto recv() -> Rc<T> {
    auto lock = _mutex.lock();
    return _msgq.pop_front().unwrap_or({});
  }
};

template <class T>
class Chan {
  using Receiver = Receiver<T>;
  using Sender = Sender<T>;

  Mutex _mutex{};
  String _topic{};

  Vec<Sender*> _senders{};
  Vec<Receiver*> _receivers{};

 public:
  explicit Chan(Str topic) : _topic{String::from(topic)} {}

  ~Chan() {
    this->disconnect();
  }

  Chan(const Chan&) = delete;
  Chan& operator=(const Chan&) = delete;

  auto topic() const -> Str {
    return _topic.as_str();
  }

 private:
  friend Receiver;
  friend Sender;

  void disconnect() {
    auto lock = _mutex.lock();
    for (auto* sender : _senders) {
      sender->_chan = nullptr;
    }
    for (auto* receiver : _receivers) {
      receiver->_chan = nullptr;
    }
  }

  void add_sender(Sender& sender) {
    auto lock = _mutex.lock();
    _senders.push(&sender);
  }

  void remove_sender(Sender& sender) {
    auto lock = _mutex.lock();
    _senders.retain([&](auto* sndr) { return sndr != &sender; });
  }

  void add_receiver(Receiver& receiver) {
    auto lock = _mutex.lock();
    _receivers.push(&receiver);
  }

  void remove_receiver(Receiver& receiver) {
    auto lock = _mutex.lock();
    _receivers.retain([&](auto* recver) { return recver != &receiver; });
  }

  void push(T value) {
    this->notify(Rc<T>::xnew(static_cast<T&&>(value)));
  }

  void notify(Rc<T> value) {
    auto lock = _mutex.lock();
    for (auto* receiver : _receivers) {
      receiver->notify(value.clone());
    }
  }
};

}  // namespace sfc::sync
