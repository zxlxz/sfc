#pragma once

#include "sfc/time.h"
#include "sfc/alloc.h"

namespace sfc::log {

enum class Level { Trace, Debug, Info, Warn, Error, Fatal };

struct Record {
  time::SystemTime time;
  Level level;
  Str message;

 public:
  auto level_str() const -> Str;
  auto time_str() const -> Str;

  void fmt(fmt::Formatter& f) const;
};

struct DynBackend {
  class Self;
  Self& _self;
  void (*_push)(Self&, Record record);
  void (*_flush)(Self&);

 public:
  template <class X>
  static auto of(X& x) -> DynBackend {
    return DynBackend{dyn::Impl{x}, dyn::Fn<&X::push>{}, dyn::Fn<&X::flush>{}};
  }

 public:
  void push(Record record) {
    return _push(_self, record);
  }

  void flush() {
    return _flush(_self);
  }
};

class Logger {
  DynBackend _backend;
  Level _level{Level::Info};

 public:
  explicit Logger(auto& backend) : _backend{DynBackend::of(backend)} {}

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
