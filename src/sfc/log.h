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
};

struct IBackend {
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
  void flush() {
    return (_meta->_flush)(_self);
  }

  void write(Record entry) {
    return (_meta->_write)(_self, entry);
  }
};

class Logger {
  Level _level{Level::Info};
  Vec<Box<IBackend&>> _backends{};

 public:
  Logger();
  ~Logger() noexcept;

  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  auto get_level() const -> Level;

  void set_level(Level level);

  void flush();

  void write_msg(Level level, Str msg);

  void write_fmt(Level level, Str fmts, const auto&... args) {
    if (level < _level) {
      return;
    }

    auto& buf = Logger::tls_buf();
    fmt::write(buf, fmts, args...);
    this->write_msg(level, buf.as_str());
  }

  void add_backend(auto backend) {
    _backends.push(Box<IBackend&>::xnew(mem::move(backend)));
  }

 private:
  static auto tls_buf() -> String&;
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
