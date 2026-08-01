#pragma once

#include "sfc/fs/file.h"
#include "sfc/log/logger.h"

namespace sfc::log {

class ConsoleBackend {
 public:
  void write(const Record& record) noexcept;
  void flush() noexcept;
};

class FileBackend {
  fs::File _file;

 public:
  FileBackend(fs::File file) noexcept;
  ~FileBackend() noexcept;

  void write(const Record& record) noexcept;
  void flush() noexcept;
};

class GlobalBackend {
  fs::File _file;

 public:
  explicit GlobalBackend() noexcept;
  ~GlobalBackend() noexcept;

  void set_file(fs::File file) noexcept;
  void write(const Record& record) noexcept;
  void flush() noexcept;
};

}  // namespace sfc::log
