#pragma once

#include "sfc/io/mod.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout {
  class Inn;

 public:
  static auto is_terminal() -> bool;
  static void flush();
  static void write_str(Str s);

  class Lock;
  static auto lock() -> Lock;
};

class Stdout::Lock {
  Inn& _inn;
  sync::ReentrantLock::Guard _lock;

 public:
  explicit Lock(Inn&);
  ~Lock() noexcept;

  void flush();
  void write_str(Str s);
};

class Stderr {
  class Inn;

 public:
  static auto is_terminal() -> bool;
  static void flush();
  static void write_str(Str s);

  class Lock;
  static auto lock() -> Lock;
};

class Stderr::Lock {
  Inn& _inn;
  sync::ReentrantLock::Guard _lock;

 public:
  explicit Lock(Inn&);
  ~Lock();

  void flush();
  void write_str(Str s);
};

template<class ...T>
void print(fmt::fmts_t<T...> fmts, const T& ...args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
}

template<class ...T>
void println(fmt::fmts_t<T...> fmts, const T& ...args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

template<class ...T>
void eprint(fmt::fmts_t<T...> fmts, const T& ...args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
}

template<class ...T>
void eprintln(fmt::fmts_t<T...> fmts, const T& ...args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
