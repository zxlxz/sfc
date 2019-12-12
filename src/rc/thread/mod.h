#pragma once

#include "rc/sync/arc.h"
#include "rc/sys.h"

namespace rc::thread {

using boxed::FnBox;

struct Thread {
  struct Inner {
    String _name;
    usize _id;
  };
};

template <class T>
struct JoinHandle {
  Thread _thread;
  sys::thread::Thread _native;
  sync::Arc<Option<T>> _result;
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
  auto spawn(F&& f) -> JoinHandle<T> {}
};

}  // namespace rc::thread
