#pragma once

#include "sfc/fs/path.h"

namespace sfc::fs {

class [[nodiscard]] File {
  friend struct OpenOptions;
  io::File _inn;

 public:
  File() noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto open(const Path& path) -> io::Result<File>;
  static auto create(const Path& path) -> io::Result<File>;

  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto read_to_end(Vec<u8>& buf, usize buf_len = 256) -> io::Result<usize>;
  auto read_to_string(String& buf, usize buf_len = 256) -> io::Result<usize>;

  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto write_all(Slice<const u8> buf) -> io::Result<usize>;
  auto write_str(Str str) -> io::Result<usize>;
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
