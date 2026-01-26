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
    if (_backends.is_empty() && level < _level) {
      return;
    }

    const auto entry = Record{level, Record::time_str(), msg};
    for (auto& backend : _backends.as_mut_slice()) {
      backend->write(entry);
    }
  }

  template <class... T>
  void write_fmt(Level level, fmt::fmts_t<T...> fmts, const T&... args) {
    if (_backends.is_empty() && level < _level) {
      return;
    }

    auto& buf = Record::tls_buf();
    fmt::write(buf, fmts, args...);
    this->write_str(level, buf.as_str());
  }
};

auto global() -> Logger&;

template <class... T>
void trace(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Trace, fmts, args...);
}

template <class... T>
void debug(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Debug, fmts, args...);
}

template <class... T>
void info(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Info, fmts, args...);
}

template <class... T>
void warn(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Warn, fmts, args...);
}

template <class... T>
void error(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Error, fmts, args...);
}

template <class... T>
void fatal(fmt::fmts_t<T...> fmts, const T&... args) {
  log::global().write_fmt(Level::Fatal, fmts, args...);
}

}  // namespace sfc::log
