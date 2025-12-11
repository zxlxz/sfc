#pragma once

#include "sfc/fs.h"
#include "sfc/log.h"

namespace sfc::log {

class FileBackend : public IBackend {
  fs::File _file = {};

 public:
  static auto create(fs::Path path) -> io::Result<FileBackend>;

  void flush() override;
  void write(Record entry) override;
};

}  // namespace sfc::log
