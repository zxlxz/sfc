#pragma once

#include "sfc/fs.h"
#include "sfc/log/mod.h"

namespace sfc::log {

class FileBackend {
  fs::File _file;

 public:
  explicit FileBackend(Str path);
  FileBackend(FileBackend&&) noexcept;
  ~FileBackend();

 public:
  void write_msg(Level level, Str msg);
};

}  // namespace sfc::log
