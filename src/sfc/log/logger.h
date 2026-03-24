#pragma once

#include "sfc/time.h"
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
  time::SystemTime time;
  Level level;
  Str message;

 public:
  auto time_str() const -> Str;
  auto level_str() const -> Str;

  void fmt(auto& f) const {
    const auto time_str = this->time_str();
    const auto level_str = this->level_str();
    fmt::write(f, "[{}] [{}] {}\n", time_str, level_str, message);
  }
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
    _backend.push({time, level, message});
  }

  template <class... T>
  void write_fmt(Level level, const fmt::fmts_t<T...>& fmts, const T&... args) {
    if (level < _level) {
      return;
    }
    const auto time = time::SystemTime::now();
    if constexpr (sizeof...(args) == 0) {
      _backend.push({time, level, {fmts._ptr, fmts._ptr}});
    } else {
      auto buf = fmt::FixedBuf<1024>{};
      fmt::write(buf, fmts, args...);
      _backend.push({time, level, Str::from_utf8(buf.as_bytes())});
    }
  }
};

}  // namespace sfc::log
