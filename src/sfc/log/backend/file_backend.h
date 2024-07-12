#pragma once

#include "sfc/fs.h"
#include "sfc/log/backend.h"

namespace sfc::log {

class FileBackend {
  fs::File _file;

 public:
  explicit FileBackend(fs::File inn);
  FileBackend(FileBackend&&) noexcept;
  ~FileBackend();

  static auto create(fs::Path path) -> io::Result<FileBackend>;

 public:
  void write_entry(Entry entry);
  auto make_log_str(Entry entry) const -> Str;
};

}  // namespace sfc::log
