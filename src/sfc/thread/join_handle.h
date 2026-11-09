#pragma once

#include "sfc/alloc.h"

struct _opaque_pthread_t;

namespace sfc::thread {

#ifdef _WIN32
using RawThread = void*;
#elif defined(__APPLE__)
using RawThread = struct _opaque_pthread_t*;
#else
using RawThread = unsigned long;
#endif

class JoinHandle {
  friend struct Builder;
  struct Inn {
    RawThread _native;
    auto* operator->();
  };
  Inn _inn;

 public:
  explicit JoinHandle(Inn inn) noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&& other) noexcept;
  JoinHandle& operator=(JoinHandle&& other) noexcept;

 public:
  void join();
};

struct Builder {
  usize stack_size = 0;

 public:
  auto spawn(auto f) -> JoinHandle {
    auto box_fn = Box<void()>::new_(mem::move(f));
    return this->spawn_unchecked(mem::move(box_fn));
  }

  auto spawn_unchecked(Box<void()>) -> JoinHandle;
};

class Scope {
  List<JoinHandle> _handles;

 public:
  Scope();
  ~Scope();
  Scope(const Scope&) = delete;
  Scope& operator=(const Scope&) = delete;

 public:
  void spawn(auto f) {
    _handles.push(Builder{}.spawn(mem::move(f)));
  }
};

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(mem::move(f));
}

}  // namespace sfc::thread
