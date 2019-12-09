#pragma once

#include "rc/alloc/mod.h"

namespace rc::boxed {

template <class T>
struct Box {
  T* _p;

  explicit Box(T* p) : _p{p} {}

  explicit Box(T val) : _p{nullptr} {
    _p = alloc::alloc<T>(1);
    ptr::write(_p, static_cast<T&&>(val));
  }

  ~Box() {
    if (_p == nullptr) {
      return;
    }
    mem::drop(*_p);
    alloc::dealloc(_p, 1);
  }

  Box(Box&& other) noexcept : _p{other._p} { other._p = nullptr; }

  static auto from_raw(T* p) -> Box { return Box{p}; }

  auto operator*() -> T& { return *_p; }

  auto into_raw() && noexcept -> T* {
    const auto ret = _p;
    _p = nullptr;
    return ret;
  }
};

template <class F>
struct FnBox;

template <class R, class... T>
struct FnBox<R(T...)> {
  using Output = R;
  using Args = Tuple<T...>;

  struct Fn {
    using Del = void (*)(Fn&);
    using XFn = R (*)(Fn&, T...);
    using Obj = unit;

    XFn _xfn;
    Del _del;
    Obj _obj;
  };

  template <class F>
  struct Fx {
    using Del = void (*)(Fx&);
    using XFn = R (*)(Fx&, T...);
    using Obj = F;

    XFn _xfn;
    Del _del;
    Obj _obj;

    Fx(F obj) noexcept : _obj{static_cast<F&&>(obj)} {
      _xfn = [](Fx& self, T... args) -> R {
        return self._obj(static_cast<T>(args)...);
      };

      _del = [](Fx& self) {
        mem::drop(self._obj);
        alloc::dealloc(&self, 1);
      };
    }
  };

  Fn* _p;

  explicit FnBox(Fn* p) noexcept : _p{p} {}

  template <class F>
  FnBox(F f) {
    const auto p = alloc::alloc<Fx<F>>(1);
    ptr::write(p, Fx<F>{rc::move(f)});
    _p = ptr::cast<Fn>(p);
  }

  ~FnBox() {
    if (_p == nullptr) {
      return;
    }
    (_p->_del)(*_p);
  }

  FnBox(FnBox&& other) noexcept : _p{other._p} { other._p = nullptr; }

  static auto from_raw(Fn* p) -> FnBox { return FnBox{p}; }

  auto into_raw() && noexcept -> Fn* {
    const auto ret = _p;
    _p = nullptr;
    return ret;
  }

  auto operator()(T... args) -> R {
    return (_p->_xfn)(*_p, static_cast<T>(args)...);
  }
};

}  // namespace rc::boxed
