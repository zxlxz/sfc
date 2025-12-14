#pragma once

#include "sfc/fs/path.h"

namespace sfc::fs {

class [[nodiscard]] File : public io::Read, public io::Write {
  friend struct OpenOptions;
  io::File _inn{};

 public:
  static auto open(Path path) noexcept-> io::Result<File>;
  static auto create(Path path) noexcept-> io::Result<File>;

  auto read(Slice<u8> buf) noexcept-> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept-> io::Result<usize>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(Path path) const noexcept -> io::Result<File>;
};

auto read(Path path) noexcept -> io::Result<Vec<u8>>;
auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<>;

}  // namespace sfc::fs
