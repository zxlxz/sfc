#pragma once

#include "chan.h"
#include "msgq.h"
#include "sched.h"

namespace sfc::cyber {

template <class T>
class Recv {
  // chan
  Rc<Chan<T>> _chan{};

  // msg
  MsgQueue<Rc<T>> _msgq{64U};

  // task
  Box<void(const T&)> _func{};
  Priority _priority{Priority::Normal};

 public:
  explicit Recv(Rc<Chan<T>> chan) noexcept : _chan{mem::move(chan)} {}

  explicit Recv(Str topic) : Recv{ChanManager<T>::instance().get(topic)} {}

  ~Recv() {
    if (!_chan) {
      return;
    }

    _chan->remove_listener({this, &Recv::_static_on_message});

    auto& sched = Sched::global();
    sched.remove_task({this, _static_run});
  }

  Recv(const Recv&) = delete;

  void set_priority(Priority val) {
    _priority = val;
  }

  void bind(auto func) {
    if (_func) {
      return;
    }

    _func = Box<void(const T&)>::xnew(mem::move(func));
    _chan->add_listener(Listener<T>{this, &_static_on_message});
  }

 private:
  static void _static_run(void* p) {
    static_cast<Recv*>(p)->run();
  }

  static void _static_on_message(void* p, Rc<T> msg) {
    static_cast<Recv*>(p)->on_message(mem::move(msg));
  }

  void run() {
    auto msg_rc = _msgq.pop();
    if (!_func || !msg_rc) {
      return;
    }
    (_func)(**msg_rc);
  }

  void on_message(Rc<T> msg) {
    static constexpr u32 TRY_TIME_MS = 20U;
    static auto& sched = Sched::global();

    _msgq.push(mem::move(msg), TRY_TIME_MS);
    sched.submit(Task{this, &_static_run}, _priority);
  }
};

}  // namespace sfc::cyber
