#pragma once

#include "sfc/log/logger.h"
#include "sfc/fs/file.h"

namespace sfc::log {

class ConsoleBackend {
 public:
  void push(Record record) noexcept;
  void flush() noexcept;
};

class FileBackend {
  fs::File _file;

 public:
  FileBackend(fs::File file) noexcept;
  ~FileBackend() noexcept;

  void push(Record record) noexcept;
  void flush() noexcept;
};

class GlobalBackend {
  fs::File _file;

 public:
  explicit GlobalBackend() noexcept;
  ~GlobalBackend() noexcept;

  void set_file(fs::File file) noexcept;
  void push(Record record) noexcept;
  void flush() noexcept;
};

class GlobalBackend;
auto global() -> Logger<GlobalBackend>&;

void trace(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Trace, fmts, args...);
}

void debug(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Debug, fmts, args...);
}

void info(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Info, fmts, args...);
}

void warn(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Warn, fmts, args...);
}

void error(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Error, fmts, args...);
}

void fatal(const fmt::Fmts& fmts, const auto&... args) {
  log::global().write_fmt(Level::Fatal, fmts, args...);
}

}  // namespace sfc::log
