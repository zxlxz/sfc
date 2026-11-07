#pragma once

#include "sfc/time.h"
#include "sfc/alloc.h"

namespace sfc::log {

enum class Level { Trace, Debug, Info, Warn, Error, Fatal };

struct Record {
  time::SystemTime _time;
  Level _level;
  Str _message;
};

struct Backend {
  struct Impl;
  struct VTbl {
    void (*push)(void*, const Record& record);
    void (*flush)(void*);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .push = [](void* x, const Record& record) { return ((X*)x)->push(record); },
      .flush = [](void* x) { return ((X*)x)->flush(); },
  };
};

struct Backend::Impl {
  const VTbl* _vtbl;
  void* _impl;

 public:
  auto push(const Record& record) -> void {
    return (_vtbl->push)(_impl, record);
  }

  void flush() {
    return (_vtbl->flush)(_impl);
  }
};

class Logger {
  Dyn<Backend> _backend;
  Level _level{Level::Info};

 public:
  explicit Logger(auto& backend) : _backend{backend} {}

 public:
  auto level() const -> Level;
  void set_level(Level level);

  void flush();
  void write_str(Level level, Str message);
  void write_fmt(Level level, fmt::Args args);
};

auto global() -> Logger&;

void trace(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Trace, {fmts, args...});
}

void debug(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Debug, {fmts, args...});
}

void info(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Info, {fmts, args...});
}

void warn(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Warn, {fmts, args...});
}

void error(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Error, {fmts, args...});
}

void fatal(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Fatal, {fmts, args...});
}

}  // namespace sfc::log
