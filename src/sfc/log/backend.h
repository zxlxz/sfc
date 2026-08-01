#pragma once

#include "sfc/fs/file.h"

namespace sfc::log {

class ConsoleBackend {
 public:
  void write(Str time_str, Str level_str, const fmt::Args& args) noexcept;
  void flush() noexcept;
};

class FileBackend {
  fs::File _file;

 public:
  FileBackend(fs::File file) noexcept;
  ~FileBackend() noexcept;

  void write(Str time_str, Str level_str, const fmt::Args& args) noexcept;
  void flush() noexcept;
};

class GlobalBackend {
  fs::File _file;

 public:
  explicit GlobalBackend() noexcept;
  ~GlobalBackend() noexcept;

  void set_file(fs::File file) noexcept;
  void write(Str time_str, Str level_str, const fmt::Args& args) noexcept;
  void flush() noexcept;
};

}  // namespace sfc::log
