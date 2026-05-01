#pragma once

#include "sfc/io/mod.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout {
  class Inn;

 public:
  class Lock {
    Inn& _inn;
    sync::ReentrantLock::Guard _lock;

   public:
    explicit Lock(Inn&);
    ~Lock() noexcept;

    void flush();
    void write_str(Str s);
  };

 public:
  static auto lock() -> Lock;
  static auto is_terminal() -> bool;

  static void flush() {
    return Stdout::lock().flush();
  }

  static void write_str(Str s) {
    return Stdout::lock().write_str(s);
  }
};

class Stderr {
  class Inn;

 public:
  class Lock {
    Inn& _inn;
    sync::ReentrantLock::Guard _lock;

   public:
    explicit Lock(Inn&);
    ~Lock();

    void flush();
    void write_str(Str s);
  };

 public:
  static auto lock() -> Lock;
  static auto is_terminal() -> bool;

  static void flush() {
    return Stderr::lock().flush();
  }

  static void write_str(Str s) {
    return Stderr::lock().write_str(s);
  }
};

void print(const fmt::Fmts& fmts, const auto&... args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
}

void println(const fmt::Fmts& fmts, const auto&... args) {
  auto out = Stdout::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

void eprint(const fmt::Fmts& fmts, const auto&... args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
}

void eprintln(const fmt::Fmts& fmts, const auto&... args) {
  auto out = Stderr::lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
