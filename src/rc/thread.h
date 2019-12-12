#pragma once

#include "rc/sys.h"
#include "rc/ffi.h"

namespace rc::thread {

using boxed::FnBox;

template<class T>
struct JoinHandle {
  sys::thread::Thread _inner;
};

struct Thread {
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

  template<class F, class R = ops::Invoke<F>::Result>
  auto spawn(F&& f) -> JoinHandle<R> {
  }
};

}  // namespace rc::thread
