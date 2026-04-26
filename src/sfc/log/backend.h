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
