#pragma once

#include "sfc/io/mod.h"
#include "sfc/sync/mutex.h"

#ifdef stdout
#undef stdout
#endif

#ifdef stderr
#undef stderr
#endif

namespace sfc::io {

class Stdout {
  class Inn;
  Inn& _inn;

 public:
  explicit Stdout();
  ~Stdout();

 public:
  class LockGuard;
  auto lock() -> LockGuard;

  auto is_terminal() -> bool;
  void flush();
  void write_str(Str s);
};

class Stderr {
  class Inn;
  Inn& _inn;

 public:
  explicit Stderr();
  ~Stderr();

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
  explicit LockGuard(Inn& inn, sync::ReentrantLock::Guard lock);
  ~LockGuard() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

class Stderr::LockGuard {
  Inn& _inn;
  sync::ReentrantLock::Guard _lock;

 public:
  explicit LockGuard(Inn& inn, sync::ReentrantLock::Guard lock);
  ~LockGuard() noexcept;

 public:
  void flush();
  void write_str(Str s);
};

auto stdout() -> Stdout&;

auto stderr() -> Stderr&;

void print(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::stdout().lock();
  fmt::write(out, fmts, args...);
}

void println(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::stdout().lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

void eprint(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::stderr().lock();
  fmt::write(out, fmts, args...);
}

void eprintln(const fmt::Fmts& fmts, const auto&... args) {
  auto out = io::stderr().lock();
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::io
