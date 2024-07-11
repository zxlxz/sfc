#pragma once

#include "path.h"
#include "sfc/io.h"

namespace sfc::fs {

class File;

struct OpenOptions {
  bool _append = false;
  bool _create = false;
  bool _create_new = false;
  bool _read = false;
  bool _write = false;
  bool _truncate = false;

 public:
  auto open(Path path) const -> io::Result<File>;
};

class File : io::File {
  friend struct OpenOptions;

  using Inn = io::File;
  using Inn::Inn;

 public:
  File() = default;
  ~File() = default;
  File(File&&) noexcept = default;
  File& operator=(File&&) noexcept = default;

  static auto open(Path path) -> io::Result<File>;
  static auto create(Path path) -> io::Result<File>;

 public:
  using Inn::operator bool;

  using Inn::read;
  using Inn::read_all;
  using Inn::read_to_string;

  using Inn::write;
  using Inn::write_fmt;
  using Inn::write_str;
};

}  // namespace sfc::fs
