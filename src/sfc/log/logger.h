#pragma once

#include "backend.h"

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

    auto buf = fmt::Buf<4096>{};
    fmt::write(buf, fmts, args...);
    this->write_msg(level, buf.as_str());
  }

  void add_backend(auto backend) {
    this->add_backend_imp(Box<IBackend&>::xnew(mem::move(backend)));
  }

  void clear_backends();

 private:
  void add_backend_imp(Box<IBackend&> backend);
};

}  // namespace sfc::log
