#pragma once

#include "rc/alloc/mod.h"

namespace rc::boxed {

template <class T>
struct Box {
  T* _p;

  explicit Box(T* p) noexcept : _p{p} {}

  ~Box() {
    if (_p == nullptr) return;
    mem::drop(*_p);
    alloc::dealloc(_p, 1);
  }

  Box(Box&& other) noexcept : _p{other._p} { other.forget(); }

  void forget() noexcept { _p = nullptr; }

  static auto from_raw(T* p) -> Box { return Box{p}; }

  static auto create(T val) -> Box {
    const auto p = alloc::alloc<T>(1);
    ptr::write(p, static_cast<T&&>(val));
    return Box{p};
  }

  auto operator*() const & noexcept -> const T& { return *_p; }
  auto operator*() & noexcept -> T& { return *_p; }

  auto operator-> () const -> const T* { return _p; }
  auto operator-> () -> T* { return _p; }

  auto into_raw() && noexcept -> T* {
    const auto p = _p;
    this->forget();
    return p;
  }
};

template <class>
struct FnBox;

template <class R, class... T>
struct FnBox<R(T...)> {
  using Output = R;

  struct Inner {
    void (*_drop)(void*);
    void (*_call)(void*, T...);
    unit _data;

    auto operator()(T... args) -> R { return _call(&_data, rc::move(args)...); }
  };
  Inner* _p;

  explicit FnBox(Inner* p) noexcept : _p{p} {}

  ~FnBox() {
    if (_p == nullptr) return;
    _p->_drop(&_p->_data);
    alloc::dealloc(_p, 1);
  }

  FnBox(FnBox&& other) noexcept : _p{other._p} { other.forget(); }

  static auto from_raw(Inner* p) -> FnBox { return FnBox{p}; }

  template <class F>
  static auto create(F f) -> FnBox {
    using U = Tuple<void (*)(F&), R (*)(F&, T...), F>;
    const auto drop = [](F& f) -> void { mem::drop(f); };
    const auto call = [](F& f, T... args) -> R { return f(rc::move(args)...); };
    const auto praw = Box<U>::create(U{drop, call, rc::move(f)}).into_raw();
    return FnBox{ptr::cast<Inner>(praw)};
  }

  void forget() noexcept { _p = nullptr; }

  auto into_raw() && -> Inner* { return _p; }

  auto operator*() noexcept -> Inner& { return *_p; }
};

}  // namespace rc::boxed
