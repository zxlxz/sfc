#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

struct Stdout {
  class Inn;

  static auto lock() -> struct StdoutLock;
  static auto is_tty() -> bool;
  static void flush();
  static void write_str(Str s);
};

struct StdoutLock {
  using LockGuard = sync::ReentrantLockGuard;

  Stdout&   _impl;
  LockGuard _lock;

 public:
  auto is_tty() const -> bool {
    return _impl.is_tty();
  }

  void flush() {
    _impl.flush();
  }

  void write_str(Str s) {
    _impl.write_str(s);
  }

  void write_fmt(const auto&... args) {
    fmt::write(_impl, args...);
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

}  // namespace sfc::io
