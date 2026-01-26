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

    template <class... T>
    void write_fmt(fmt::fmts_t<T...> fmts, const T&... args) {
      fmt::Fmter{*this}.write_fmt(fmts, args...);
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

    template <class... T>
    void write_fmt(fmt::Fmts<T...> fmts, const T&... args) {
      fmt::Fmter{*this}.write_fmt(fmts, args...);
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

template <class... T>
void print(fmt::fmts_t<T...> fmts, const T&... args) {
  auto out = Stdout::lock();
  out.write_fmt(fmts, args...);
}

template <class... T>
void println(fmt::fmts_t<T...> fmts, const T&... args) {
  auto out = Stdout::lock();
  out.write_fmt(fmts, args...);
  out.write_str("\n");
}

template <class... T>
void eprint(fmt::fmts_t<T...> fmts, const T&... args) {
  auto out = Stderr::lock();
  out.write_fmt(fmts, args...);
}

template <class... T>
void eprintln(fmt::fmts_t<T...> fmts, const T&... args) {
  auto out = Stderr::lock();
  out.write_fmt(fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
