#pragma once

#include "rc/alloc/mod.h"

namespace rc::boxed {

template <class T>
struct Box {
  T* _0;

  explicit Box(T* p) noexcept : _0{p} {}

  explicit Box(T val) : _0{alloc::alloc<T>(1)} {
    ptr::write(_0, static_cast<T&&>(val));
  }

  ~Box() {
    if (_0 == nullptr) return;
    mem::drop(*_0);
    alloc::dealloc(_0, 1);
  }

  Box(Box&& other) noexcept : _0{other._0} { other._0 = nullptr; }

  static auto from_raw(T* p) -> Box { return Box{p}; }

  auto operator*() const & noexcept -> T& { return *_0; }
  auto operator*() & noexcept -> T& { return *_0; }
  auto operator*() && noexcept -> T { return static_cast<T&&>(*_0); }

  auto operator-> () const -> const T* { return _0; }
  auto operator-> () -> T* { return _0; }

  auto into_raw() && noexcept -> T* {
    const auto ret = _0;
    _0 = nullptr;
    return ret;
  }
};

template<class T>
Box(T) -> Box<T>;

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

  Fn* _0;

  explicit FnBox(Fn* p) noexcept : _0{p} {}

  template <class F>
  FnBox(F f) {
    const auto p = alloc::alloc<Fx<F>>(1);
    ptr::write(p, Fx<F>{rc::move(f)});
    _0 = ptr::cast<Fn>(p);
  }

  ~FnBox() {
    if (_0 == nullptr) {
      return;
    }
    (_0->_del)(*_0);
  }

  FnBox(FnBox&& other) noexcept : _0{other._0} { other._0 = nullptr; }

  static auto from_raw(Fn* p) -> FnBox { return FnBox{p}; }

  auto into_raw() && noexcept -> Fn* {
    const auto ret = _0;
    _0 = nullptr;
    return ret;
  }

  auto operator()(T... args) -> R {
    return (_0->_xfn)(*_0, static_cast<T>(args)...);
  }
};

}  // namespace rc::boxed

namespace rc {
using rc::boxed::Box;
using rc::boxed::FnBox;
}  // namespace rc
