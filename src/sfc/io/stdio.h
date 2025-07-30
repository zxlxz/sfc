#pragma once

#include "sfc/io/error.h"

namespace sfc::io {

class Stdout {
  class Inn;

 public:
  static auto is_tty() -> bool;
  static void flush();
  static void write_str(Str s);

  class Lock;
  static auto lock() -> Lock;
};

class Stdout::Lock {
  using Guard = sync::ReentrantLock::Guard;

  Inn& _inn;
  Guard _lock;

 public:
  explicit Lock(Inn& inn);
  ~Lock();

  void flush();
  void write_str(Str s);

  void write_fmt(const auto&... args) {
    fmt::Fmter{*this}.write_fmt(args...);
  }
};

void print(const auto&... args) {
  auto out = Stdout{}.lock();
  out.write_fmt(args...);
}

void println(const auto&... args) {
  auto out = Stdout{}.lock();
  out.write_fmt(args...);
  out.write_str("\n");
}

}  // namespace sfc::io
