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

  void write_fmt(Level level, const auto& args) {
    if (level < _level) {
      return;
    }

    char buf[1024];
    auto out = fmt::SBuf{buf};
    fmt::Formatter{out}.write_val(args);
    this->write_str(level, out.as_str());
  }
};

}  // namespace sfc::log
