#pragma once

#include "sfc/fs/file.h"
#include "sfc/log/logger.h"

namespace sfc::log {

class ConsoleBackend {
 public:
  void push(const Record& record);
  void flush();
};

class FileBackend {
  fs::File _file;

 public:
  explicit FileBackend(fs::File file);
  ~FileBackend();

  FileBackend(FileBackend&&) noexcept = default;
  FileBackend& operator=(FileBackend&&) noexcept = default;

 public:
  void push(const Record& record);
  void flush();
};

class GlobalBackend {
  Option<fs::File> _file;

 public:
  void set_file(fs::File file) noexcept;

  void push(const Record& record);
  void flush();
};

}  // namespace sfc::log
