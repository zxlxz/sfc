#pragma once

#include "rc/sync/arc.h"
#include "rc/sys.h"

namespace rc::thread {

namespace imp = rc::sys::thread;

template <class T>
struct JoinHandle {
  imp::Thread _thr;
  boxed::Box<Option<T>> _res;
};

struct Builder {
  String _name;
  usize _stack_size;

  auto name(String name) const -> Builder {
    return Builder{rc::move(name), _stack_size};
  }

  auto stack_size(usize stack_size) && -> Builder {
    return Builder{rc::move(_name), stack_size};
  }

  template <class F, class T = typename ops::Invoke<F>::Result>
  auto spawn(F f) -> JoinHandle<T> {
    auto res = boxed::Box<Option<T>>::create(Option<T>{});
    auto fun = [f = rc::move(f), p = res._p] {
      try {
        if constexpr (rc::is_same<T, void>()) {
          f();
          ptr::write(p, Option<T>{unit{}});
        } else {
          auto v = f();
          ptr::write(p, Option<T>{rc::move(v)});
        }
      } catch (...) {
      }
    };
    auto thr = imp::Thread::spawn(boxed::FnBox<void()>(rc::move(fun)));
    return JoinHandle<T>{rc::move(thr), rc::move(res)};
  }
};

}  // namespace rc::thread
