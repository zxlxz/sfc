#pragma once

#include "sfc/io/read.h"
#include "sfc/io/write.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout {
  class Inn;
  using LockGuard = sync::ReentrantLock::Guard;

 public:
  class Lock {
    Inn& _inn;
    LockGuard _lock;

   public:
    explicit Lock(Inn&);
    ~Lock() noexcept;

    auto is_tty() -> bool;
    void flush();
    void write_str(Str s);

    void write_fmt(const auto&... args) {
      fmt::Fmter{*this}.write_fmt(args...);
    }
  };

  static auto lock() -> Lock;

  static auto is_tty() -> bool {
    return Stdout::lock().is_tty();
  }

  static void flush() {
    return Stdout::lock().flush();
  }

  static void write_str(Str s) {
    return Stdout::lock().write_str(s);
  }
};

class Stderr {
  class Inn;
  using LockGuard = sync::ReentrantLock::Guard;

 public:
  class Lock {
    Inn& _inn;
    LockGuard _lock;

   public:
    explicit Lock(Inn&);
    ~Lock();

    static auto is_tty() -> bool;
    void flush();
    void write_str(Str s);

    void write_fmt(const auto&... args) {
      fmt::Fmter{*this}.write_fmt(args...);
    }
  };

  static auto lock() -> Lock;

  static auto is_tty() -> bool {
    return Stderr::lock().is_tty();
  }

  static void flush() {
    return Stderr::lock().flush();
  }

  static void write_str(Str s) {
    return Stderr::lock().write_str(s);
  }
};

void print(Str fmt, const auto&... args) {
  auto out = Stdout::lock();
  out.write_fmt(fmt, args...);
}

void println(Str fmt, const auto&... args) {
  auto out = Stdout::lock();
  out.write_fmt(fmt, args...);
  out.write_str("\n");
}

void eprint(Str fmt, const auto&... args) {
  auto out = Stderr::lock();
  out.write_fmt(fmt, args...);
}

void eprintln(Str fmt, const auto&... args) {
  auto out = Stderr::lock();
  out.write_fmt(fmt, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
