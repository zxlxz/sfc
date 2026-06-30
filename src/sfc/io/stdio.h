#pragma once

#include "sfc/io/mod.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class StdoutImpl;
class StderrImpl;

class StdoutLock {
  sync::ReentrantLock::Guard _lock;

 public:
  explicit StdoutLock(StdoutImpl&);
  ~StdoutLock() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

class StderrLock {
  sync::ReentrantLock::Guard _lock;

 public:
  explicit StderrLock(StderrImpl&);
  ~StderrLock() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

class Stdout {
 public:
  static auto lock() -> StdoutLock;
  static auto is_terminal() -> bool;

  static void flush() {
    return Stdout::lock().flush();
  }

  static void write_str(Str s) {
    return Stdout::lock().write_str(s);
  }
};

class Stderr {
 public:
  static auto lock() -> StderrLock;
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

namespace sfc::fmt {
extern template struct Formatter<io::Stdout>;
extern template struct Formatter<io::StdoutLock>;

extern template struct Formatter<io::Stderr>;
extern template struct Formatter<io::StderrLock>;
}  // namespace sfc::fmt
