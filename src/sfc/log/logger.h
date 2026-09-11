#pragma once

#include "sfc/time.h"
#include "sfc/alloc.h"

namespace sfc::log {

enum class Level { Trace, Debug, Info, Warn, Error, Fatal };

struct Record {
  time::SystemTime _time;
  Level _level;
  fmt::Args _args;
};

class DynBackend final {
  void* _self;
  void (*_write)(void*, const Record& record);
  void (*_flush)(void*);

 public:
  template <trait::not_<DynBackend> X>
  DynBackend(X& x)
      : _self{&x}
      , _write{[](void* p, const Record& record) { ((X*)p)->write(record); }}
      , _flush{[](void* p) { ((X*)p)->flush(); }} {}

 public:
  void write(const Record& record) {
    return _write(_self, record);
  }

  void flush() {
    return _flush(_self);
  }
};

class Logger {
  DynBackend _backend;
  Level _level{Level::Info};

 public:
  explicit Logger(DynBackend backend) : _backend{backend} {}

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
