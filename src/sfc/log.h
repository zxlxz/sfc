#pragma once

#include "sfc/alloc.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warning,
  Error,
  Fatal,
};

struct Entry {
  Level level;
  Str time;
  Str msg;
};

class IBackend : Box<IBackend&> {
  friend Box<IBackend&>;
  struct Meta : Box<IBackend&>::Meta {
    void (*_flush)(void*) = nullptr;
    void (*_write_entry)(void*, Entry) = nullptr;

    template <class X>
    Meta(X* x) : Box<IBackend&>::Meta{x} {
      _flush = ([](void* p) { static_cast<X*>(p)->flush(); });
      _write_entry = ([](void* p, Entry entry) { static_cast<X*>(p)->write_entry(entry); });
    }
  };

 public:
  void flush() {
    return (static_cast<const Meta*>(_meta)->_flush)(_self);
  }

  void write_entry(Entry entry) {
    return (static_cast<const Meta*>(_meta)->_write_entry)(_self, entry);
  }
};

class Logger {
  Level _level{Level::Info};
  Vec<Box<IBackend&>> _backends{};

 public:
  Logger();
  ~Logger();

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

void warning(const auto&... args) {
  log::global().write_fmt(Level::Warning, args...);
}

void error(const auto&... args) {
  log::global().write_fmt(Level::Error, args...);
}

void fatal(const auto&... args) {
  log::global().write_fmt(Level::Fatal, args...);
}

}  // namespace sfc::log
