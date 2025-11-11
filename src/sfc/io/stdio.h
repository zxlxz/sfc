#pragma once

#include "sfc/io/read.h"
#include "sfc/io/write.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout {
  class Inn;
  using Guard = sync::ReentrantLock::Guard;

 public:
  class Lock {
    Inn& _inn;
    Guard _lock;

   public:
    explicit Lock();
    ~Lock() noexcept;

    Lock(const Lock&) noexcept = delete;
    Lock& operator=(const Lock&) noexcept = delete;

    static auto is_tty() -> bool;
    void flush();
    void write_str(Str s);

    void write_fmt(const auto&... args) {
      fmt::Fmter{*this}.write_fmt(args...);
    }
  };

  static auto lock() -> Lock {
    return Lock{};
  }

  static auto is_tty() -> bool {
    return Lock::is_tty();
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
  using Guard = sync::ReentrantLock::Guard;

 public:
  class Lock {
    Inn& _inn;
    Guard _lock;

   public:
    explicit Lock();
    ~Lock() noexcept;

    Lock(const Lock&) noexcept = delete;
    Lock& operator=(const Lock&) noexcept = delete;

    static auto is_tty() -> bool;
    void flush();
    void write_str(Str s);

    void write_fmt(const auto&... args) {
      fmt::Fmter{*this}.write_fmt(args...);
    }
  };

  static auto lock() -> Lock {
    return Lock{};
  }

  static auto is_tty() -> bool {
    return Lock::is_tty();
  }

  static void flush() {
    return Stderr::lock().flush();
  }

  static void write_str(Str s) {
    return Stderr::lock().write_str(s);
  }
};

void print(const auto&... args) {
  auto out = Stdout::lock();
  out.write_fmt(args...);
}

void println(const auto&... args) {
  auto out = Stdout::lock();
  out.write_fmt(args...);
  out.write_str("\n");
}

void eprint(const auto&... args) {
  auto out = Stderr::lock();
  out.write_fmt(args...);
}

void eprintln(const auto&... args) {
  auto out = Stderr::lock();
  out.write_fmt(args...);
  out.write_str("\n");
}

}  // namespace sfc::io
