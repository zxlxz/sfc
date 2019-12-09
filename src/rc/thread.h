#pragma once

#include "rc/sys.h"
#include "rc/ffi.h"

namespace rc::thread {

using boxed::FnBox;

struct JoinHandle {
  sys::thread::Thread _inner;
};

struct Thread {
  sys::thread::Thread _inner;

  pub auto _spawn(FnBox<void()> callback) -> Thread;
  pub auto _join() -> void;

  pub static auto yield_now() -> void;
  pub static auto sleep(time::Duration dur) -> void;
};

}  // namespace rc::thread
