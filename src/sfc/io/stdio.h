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

  auto is_terminal() -> bool;
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

  auto is_terminal() -> bool;
  void flush();
  void write_str(Str s);
};

void print(fmt::Fmts fmts, const auto&... args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
}

void println(fmt::Fmts fmts, const auto&... args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

void eprint(fmt::Fmts fmts, const auto&... args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
}

void eprintln(fmt::Fmts fmts, const auto&... args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
