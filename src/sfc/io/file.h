#pragma once

#include "mod.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class File {
 protected:
  fd_t _fd = fd_t(-1);

 public:
  File() noexcept;
  ~File();
  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(fd_t fd) -> File;

 public:
  explicit operator bool() const;

  auto read(Slice<u8> buf) -> Result<usize>;
  auto write(Slice<const u8> buf) -> Result<usize>;

  auto read_all(Vec<u8>& buf, usize buf_len = 256) -> Result<usize>;
  auto read_to_string(String& buf) -> Result<usize>;

  auto write_all(Slice<const u8> buf) -> Result<usize>;
  auto write_str(Str str) -> Result<usize>;
};

}  // namespace sfc::io
