#pragma once

#include "backend.h"

namespace sfc::log {

class Logger {
  Level               _level{Level::Info};
  Vec<Box<IBackend&>> _backends{};

 public:
  Logger();
  ~Logger();

  Logger(Logger&&) noexcept;
  Logger& operator=(Logger&&) noexcept;

  auto get_level() const -> Level;

  void set_level(Level level);

  void flush();

  void write_msg(Level level, Str msg);

  void write_fmt(Level level, Str fmts) {
    if (level < _level) {
      return;
    }
    this->write_msg(level, fmts);
  }

  void write_fmt(Level level, Str fmts, const auto&... args) {
    if (level < _level) {
      return;
    }

    if constexpr (sizeof...(args) == 0) {
      this->write_msg(level, fmts);
    } else {
      const auto msg = string::format(fmts, args...);
      this->write_msg(level, msg.as_str());
    }
  }

  void add_backend(auto backend) {
    _backends.push(boxed::box(mem::move(backend)));
  }

  void clear_backends() {
    _backends.clear();
  }
};

}  // namespace sfc::log
