#pragma once

#include "sfc/collections.h"

namespace sfc::cyber {

template <class T>
struct Listener {
  void* _self;
  void (*_func)(void*, Rc<T>);

 public:
  auto operator==(const Listener& rhs) const -> bool {
    return _self == rhs._self && _func == rhs._func;
  }
};

template <class T>
class Chan {
  String _topic;
  Vec<Listener<T>> _listeners = {};

 public:
  explicit Chan(Str topic) : _topic{topic} {}

  ~Chan() {}

  Chan(const Chan&) = delete;

  auto topic() const -> Str {
    return _topic;
  }

  void push(T msg) {
    this->transmit(static_cast<T&&>(msg));
  }

  void add_listener(Listener<T> listener) {
    _listeners.push(listener);
  }

  auto remove_listener(Listener<T> listener) {
    _listeners.retain([&](auto& x) { return !(x == listener); });
  }

 private:
  void transmit(T msg) {
    auto rc = Rc<T>::xnew(mem::move(msg));
    this->dispatch(mem::move(rc));
  }

  void dispatch(Rc<T> msg) {
    const auto cnt = _listeners.len();
    if (cnt == 0) {
      return;
    }

    for (auto& x : _listeners.as_mut_slice()) {
      (*x._func)(x._self, msg.clone());
    }
  }
};

template <class T>
class ChanManager {
  VecMap<String, Rc<Chan<T>>> _chans = {};

  ChanManager() = default;
  ~ChanManager() = default;
  ChanManager(const ChanManager&) = delete;

 public:
  static auto instance() -> ChanManager& {
    static ChanManager res{};
    return res;
  }

  auto get(Str topic) -> Rc<Chan<T>> {
    auto res = _chans.get_mut(topic);
    if (!res) {
      _chans.insert(String{topic}, Rc<Chan<T>>::xnew(topic));
      res = _chans.get_mut(topic);
    }
    return (*res).clone();
  }
};

}  // namespace sfc::cyber
