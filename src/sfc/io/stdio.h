#pragma once

#include "sfc/io/mod.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout {
  struct Inn;

 public:
  class LockGuard;
  auto lock() -> LockGuard;

  auto is_terminal() -> bool;
  void flush();
  void write_str(Str s);
};

class Stderr {
  struct Inn;

 public:
  class LockGuard;
  auto lock() -> LockGuard;

  auto is_terminal() -> bool;
  void flush();
  void write_str(Str s);
};

class Stdout::LockGuard {
  Inn& _inn;
  sync::ReentrantLock::Guard _lock;

 public:
  LockGuard(Inn& inn, sync::ReentrantLock::Guard lock);
  ~LockGuard() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

class Stderr::LockGuard {
  Inn& _inn;
  sync::ReentrantLock::Guard _lock;

 public:
  LockGuard(Inn& inn, sync::ReentrantLock::Guard lock);
  ~LockGuard() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

void print(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::Stdout().lock();
  fmt::write(out, fmts, args...);
}

void println(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::Stdout().lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

void eprint(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::Stderr().lock();
  fmt::write(out, fmts, args...);
}

void eprintln(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::Stderr().lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
