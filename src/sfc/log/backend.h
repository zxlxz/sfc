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
  static auto create(fs::Path path) -> io::Result<FileBackend>;

 public:
  void push(const Record& record);
  void flush();
};

class GlobalBackend {
  fs::File _file;

 public:
  auto set_path(fs::Path path) noexcept -> io::Result<>;

  void push(const Record& record);
  void flush();
};

}  // namespace sfc::log
