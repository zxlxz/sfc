#pragma once

#include "sfc/fs/path.h"

namespace sfc::fs {

class [[nodiscard]] File : io::File {
  using Inn = io::File;

 public:
  explicit File(io::File inn) : Inn{static_cast<Inn&&>(inn)} {}
  ~File() = default;

  File(File&&) noexcept = default;
  File& operator=(File&&) noexcept = default;

  using Inn::read;
  using Inn::read_all;
  using Inn::read_to_string;

  using Inn::write;
  using Inn::write_all;
  using Inn::write_str;

  static auto open(const Path& path) -> io::Result<File>;
  static auto create(const Path& path) -> io::Result<File>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(const Path& path) const -> io::Result<File>;
};

}  // namespace sfc::fs
