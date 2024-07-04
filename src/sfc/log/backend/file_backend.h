#pragma once

#include "sfc/fs.h"
#include "sfc/log/mod.h"

namespace sfc::log {

class FileBackend : public IBackend {
 public:
  explicit FileBackend(Str path);
  FileBackend(FileBackend&&) noexcept;
  ~FileBackend();

  void write_msg(Level level, Str msg) override;

 private:
  fs::File _file;
};

}  // namespace sfc::log
