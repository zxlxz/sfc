#pragma once

#include "sfc/fs.h"
#include "sfc/log/backend.h"

namespace sfc::log {

class FileBackend {
  static constexpr usize BUFF_SIZE = 4096U;

  fs::File _file;

 public:
  explicit FileBackend(fs::File&& inn);

  ~FileBackend();

  FileBackend(FileBackend&&) noexcept = default;

  FileBackend& operator=(FileBackend&&) noexcept = default;

  static auto create(fs::Path path) -> io::Result<FileBackend>;

  void flush();

  void write_entry(Entry entry);
};

}  // namespace sfc::log
