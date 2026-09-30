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

struct Backend {
  class Dyn;
};

class Backend::Dyn {
  struct VTbl {
    void (*write)(void*, const Record& record);
    void (*flush)(void*);

    template <class X>
    static auto of() -> const VTbl& {
      static const auto vtbl = VTbl{
          .write = [](void* x, const Record& record) { return ((X*)(x))->write(record); },
          .flush = [](void* x) { return ((X*)(x))->flush(); },
      };
      return vtbl;
    }
  };
  const VTbl& _vtbl;
  void* _self;

 public:
  template <trait::not_<Dyn> X>
  Dyn(X& x) : _vtbl{VTbl::of<X>()}, _self{&x} {}

  void write(const Record& record) {
    return (_vtbl.write)(_self, record);
  }

  void flush() {
    return (_vtbl.flush)(_self);
  }
};

class Logger {
  Backend::Dyn _backend;
  Level _level{Level::Info};

 public:
  explicit Logger(Backend::Dyn backend) : _backend{backend} {}

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
