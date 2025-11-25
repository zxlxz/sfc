#pragma once

#include "sfc/fs/path.h"

namespace sfc::fs {

class [[nodiscard]] File : public io::Read, public io::Write {
  friend struct OpenOptions;
  io::File _inn;

 public:
  File() noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto open(Path path) -> io::Result<File>;
  static auto create(Path path) -> io::Result<File>;

  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(Path path) const -> io::Result<File>;
};

auto read(Path path) -> io::Result<Vec<u8>>;
auto write(Path path, Slice<const u8> buf) -> io::Result<>;

}  // namespace sfc::fs
