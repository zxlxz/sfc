#pragma once

#include "sfc/fs.h"
#include "sfc/log.h"

namespace sfc::log {

class FileBackend {
  fs::File _file;

 public:
  static auto create(fs::Path path) -> io::Result<FileBackend>;

  void flush();
  void write(Record entry);
};

}  // namespace sfc::log
