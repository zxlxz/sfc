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
  auto time_str() const -> Str;
  auto level_str() const -> Str;
};

template <class Backend>
class Logger {
  Backend& _backend;
  Level _level{Level::Info};

 public:
  Logger(Backend& backend) noexcept : _backend{backend} {}
  ~Logger() noexcept {}
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  auto level() const -> Level {
    return _level;
  }

  void set_level(Level level) {
    _level = level;
  }

  auto backend() -> Backend& {
    return _backend;
  }

  void flush() {
    _backend.flush();
  }

  void write_str(Level level, Str message) {
    if (level < _level) {
      return;
    }

    const auto time = time::SystemTime::now();
    _backend.push(Record{time, level, message});
  }

  template <class... T>
  void write_fmt(Level level, const fmt::fmts_t<T...>& fmts, const T&... args) {
    if (level < _level) {
      return;
    }

    auto buf = fmt::FixedBuf<1024>{};
    fmt::write(buf, fmts, args...);
    this->write_str(level, buf.as_str());
  }
};

}  // namespace sfc::log
