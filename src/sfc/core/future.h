#pragma once

#include "sfc/core/option.h"
#include "sfc/core/trait.h"

namespace sfc::future {

enum class State {
  Pending,
  Yield,
  Ready,
};

struct Suspend {
  void await_resume() const noexcept {}
  void await_suspend(auto) const noexcept {}
  auto await_ready() const noexcept -> bool {
    return false;
  }
};

struct Frame {
  void* _addr = nullptr;

 public:
  auto done() const noexcept -> bool {
    return __builtin_coro_done(_addr);
  }

  void resume() const noexcept {
    return __builtin_coro_resume(_addr);
  }

  void destroy() const noexcept {
    return __builtin_coro_destroy(_addr);
  }

  template <usize ALIGN>
  auto promise() const noexcept -> void* {
    return __builtin_coro_promise(_addr, ALIGN, false);
  }

  template <usize ALIGN>
  static auto from_promise(void* promise) noexcept -> Frame {
    return Frame{__builtin_coro_promise(promise, ALIGN, true)};
  }
};

template <class T>
struct Poll {
  State _state = State::Pending;
  T& _value;

 public:
  auto is_ready() const noexcept -> bool {
    return _state == State::Ready;
  }

  auto is_pending() const noexcept -> bool {
    return _state == State::Pending;
  }

  auto operator*() const -> const T& {
    sfc::expect(_state != State::Pending, "Poll::operator*: cannot dereference a pending poll");
    return _value;
  }

  auto operator*() -> T& {
    sfc::expect(_state != State::Pending, "Poll::operator*: cannot dereference a pending poll");
    return _value;
  }
};

template <>
struct Poll<void> {
  State _state = State::Pending;

 public:
  explicit Poll(State state) noexcept : _state{state} {}

  auto is_ready() const noexcept -> bool {
    return _state == State::Ready;
  }

  auto is_pending() const noexcept -> bool {
    return _state == State::Pending;
  }
};

template <class T>
class Promise;

template <class T>
class Future {
  friend Promise<T>;
  Frame _frame;

 public:
  explicit Future() = default;

  explicit Future(Promise<T>& p) : _frame{Frame::from_promise<alignof(Promise<T>)>(&p)} {}

  ~Future() {
    _frame.destroy();
  }

  auto poll() -> Poll<T> {
    if (!_frame.done()) {
      _frame.resume();
    }
    const auto promise = static_cast<Promise<T>*>(_frame.promise<alignof(Promise<T>)>());
    if constexpr (__is_void(T)) {
      return Poll<T>{promise->_state};
    } else {
      return Poll<T>{promise->_state, promise->_value};
    }
  }

  struct Await {
    Frame _frame;

   public:
    auto await_ready() const noexcept -> bool {
      return _frame.done();
    }

    void await_suspend(Frame frame) const noexcept {
      (void)frame;
      return _frame.resume();
    }

    auto await_resume() -> T {
      if constexpr (!__is_void(T)) {
        const auto promise = static_cast<Promise<T>*>(_frame.promise<alignof(Promise<T>)>());
        return static_cast<T&&>(promise->_value);
      }
    }
  };

  auto operator co_await() const noexcept -> Await {
    return Await{_frame};
  }
};

template <class T>
class Promise {
  friend Future<T>;

  State _state = State::Pending;
  union {
    T _value;
  };

 public:
  explicit Promise() noexcept {}

  ~Promise() {
    _state != State::Pending ? _value.~T() : void();
    _state = State::Pending;
  }

  auto get_return_object() -> Future<T> {
    return Future<T>{*this};
  }

  auto initial_suspend() noexcept -> Suspend {
    return {};
  }

  auto final_suspend() noexcept -> Suspend {
    return {};
  }

  auto yield_value(T val) noexcept -> Suspend {
    // NOLINTNEXTLINE (clang-analyzer-core.uninitialized.Branch)
    if (_state != State::Pending) {
      mem::drop(_value);
    }
    ptr::write(&_value, mem::move(val));
    _state = State::Yield;
    return {};
  }

  void return_value(T val) noexcept {
    if (_state != State::Pending) {
      mem::drop(_value);
    }
    ptr::write(&_value, mem::move(val));
    _state = State::Ready;
  }

  void unhandled_exception() noexcept {}
};

template <>
class Promise<void> {
  friend Future<void>;
  State _state = State::Pending;

 public:
  auto get_return_object() noexcept -> Future<void> {
    return Future{*this};
  }

  auto initial_suspend() const noexcept -> Suspend {
    return {};
  }

  auto final_suspend() const noexcept -> Suspend {
    return {};
  }

  void return_void() noexcept {
    _state = State::Ready;
  }

  void unhandled_exception() noexcept {}
};

}  // namespace sfc::future

namespace SFC_STD {

template <class T, class... Args>
struct coroutine_traits;

template <class T>
struct coroutine_handle;

template <class T, class... Args>
struct coroutine_traits<sfc::future::Future<T>, Args...> {
  using promise_type = sfc::future::Promise<T>;
};

template <class T>
struct coroutine_handle<sfc::future::Promise<T>> {
  void* _frame = nullptr;

  static auto from_address(void* addr) noexcept -> coroutine_handle {
    return coroutine_handle{addr};
  }

  operator sfc::future::Frame() const noexcept {
    return sfc::future::Frame{_frame};
  }
};

}  // namespace SFC_STD

namespace sfc {
using future::Future;
}  // namespace sfc
