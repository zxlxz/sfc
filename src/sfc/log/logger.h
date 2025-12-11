#pragma once

#include "sfc/log/backend.h"

namespace sfc::log {

class Logger {
  Level _level{Level::Info};
  Vec<Box<IBackend>> _backends = {};

 public:
  auto level() const -> Level {
    return _level;
  }

  void set_level(Level level) {
    _level = level;
  }

  template <class B>
  void add_backend(B backend) {
    _backends.push(box(static_cast<B&&>(backend)).template cast<IBackend>());
  }

  void flush() {
    for (auto& backend : _backends) {
      backend->flush();
    }
  }

  void write_str(Level level, Str msg) {
    if (_backends.is_empty() && level < _level) {
      return;
    }

    const auto entry = Record{level, Record::time_str(), msg};
    for (auto& backend : _backends) {
      backend->write(entry);
    }
  }

  void write_fmt(Level level, Str fmts, const auto&... args) {
    if (_backends.is_empty() && level < _level) {
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
