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
    for (auto& backend : _backends.as_mut_slice()) {
      backend->flush();
    }
  }

  void write_str(Level level, Str msg) {
    if (_backends.is_empty() || level < _level) {
      return;
    }

    const auto entry = Record{level, Record::time_str(), msg};
    for (auto& backend : _backends.as_mut_slice()) {
      backend->write(entry);
    }
  }

  void write_fmt(Level level, fmt::Fmts fmts, const auto&... args) {
    if (_backends.is_empty() && level < _level) {
      return;
    }

    auto& buf = Record::tls_buf();
    fmt::write(buf, fmts, args...);
    this->write_str(level, buf.as_str());
  }
};

auto global() -> Logger&;

void trace(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Trace, fmts, args...);
}

void debug(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Debug, fmts, args...);
}

void info(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Info, fmts, args...);
}

void warn(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Warn, fmts, args...);
}

void error(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Error, fmts, args...);
}

void fatal(fmt::Fmts fmts, const auto&... args) {
  log::global().write_fmt(Level::Fatal, fmts, args...);
}

}  // namespace sfc::log
