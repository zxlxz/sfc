#pragma once

#include "sfc/alloc.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

struct Record {
  Level level;
  Str time;
  Str msg;

 public:
  static auto tls_buf() -> String&;
  static auto time_str() -> Str;
};

class Backend {
  template <class T, class A>
  friend class boxed::Box;

  struct Meta {
    void (*_flush)(void*) = nullptr;
    void (*_write)(void*, Record) = nullptr;

    template <class X>
    static auto from(const X&) -> Meta {
      const auto flush = [](void* p) { static_cast<X*>(p)->flush(); };
      const auto write = [](void* p, Record r) { static_cast<X*>(p)->write(r); };
      return {flush, write};
    }
  };

  const Meta* _meta = nullptr;
  void* _self = nullptr;

 public:
  static auto from(auto& impl) -> Backend {
    static const auto meta = Meta::from(impl);
    auto res = Backend{};
    res._meta = &meta;
    res._self = &impl;
    return res;
  }

  explicit operator bool() const {
    return _self != nullptr;
  }

  void flush() {
    return (_meta->_flush)(_self);
  }

  void write(Record entry) {
    return (_meta->_write)(_self, entry);
  }
};

class Logger {
  Level _level{Level::Info};
  Backend _backend = {};

 public:
  Logger() = default;
  ~Logger() noexcept = default;

  Logger(Logger&&) = default;
  Logger& operator=(Logger&&) = default;

  auto level() const -> Level {
    return _level;
  }

  void set_level(Level level) {
    _level = level;
  }

  void set_backend(auto& backend) {
    _backend = Backend::from(backend);
  }

  void flush() {
    _backend ? _backend.flush() : void();
  }

  void write_str(Level level, Str msg) {
    if (!_backend || level < _level) {
      return;
    }

    const auto entry = Record{level, Record::time_str(), msg};
    _backend.write(entry);
  }

  void write_fmt(Level level, Str fmts, const auto&... args) {
    if (!_backend || level < _level) {
      return;
    }

    auto& buf = Record::tls_buf();
    fmt::write(buf, fmts, args...);
    this->write_str(level, buf.as_str());
  }
};

auto global() -> Logger&;

void trace(const auto&... args) {
  log::global().write_fmt(Level::Trace, args...);
}

void debug(const auto&... args) {
  log::global().write_fmt(Level::Debug, args...);
}

void info(const auto&... args) {
  log::global().write_fmt(Level::Info, args...);
}

void warn(const auto&... args) {
  log::global().write_fmt(Level::Warn, args...);
}

void error(const auto&... args) {
  log::global().write_fmt(Level::Error, args...);
}

void fatal(const auto&... args) {
  log::global().write_fmt(Level::Fatal, args...);
}

}  // namespace sfc::log
