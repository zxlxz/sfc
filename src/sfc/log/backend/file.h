#pragma once

#include "sfc/fs.h"
#include "sfc/log.h"

namespace sfc::log {

class FileBackend {
  fs::File _file;

 public:
  explicit FileBackend(fs::File&& inn);
  ~FileBackend() noexcept;

  FileBackend(FileBackend&&) noexcept = default;
  FileBackend& operator=(FileBackend&&) noexcept = default;

  static auto create(fs::Path path) -> io::Result<FileBackend>;

  void flush();

  void write(Record entry);
};

}  // namespace sfc::log
