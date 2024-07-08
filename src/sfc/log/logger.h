#pragma once

#include "mod.h"

namespace sfc::log {

class Logger {
  Level _level = Level::Info;
  Vec<Box<IBackend&>> _backends;

 public:
  Logger();
  Logger(Logger&&) noexcept;
  ~Logger();

  Logger& operator=(Logger&&) noexcept;

  auto get_level() const -> Level;

  void set_level(Level level);

  void write_msg(Level level, Str msg);

  void write_fmt(Level level, Str fmts, const auto&... args) {
    if (level < _level) {
      return;
    }

    auto& sbuf = Logger::get_tls_sbuf();
    fmt::write(sbuf, fmts, args...);
    this->write_msg(level, sbuf);
  }

  void clear_backends();

  void add_console_backend();

  void add_file_backend(Str path);

  void add_backend(Box<IBackend&> backend);

 private:
  static auto get_tls_sbuf() -> String&;
};

}  // namespace sfc::log
