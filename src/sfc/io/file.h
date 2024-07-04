#pragma once

#include "sfc/io/error.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class File {
 protected:
  fd_t _fd;

 public:
  explicit File(fd_t fd);
  ~File();

  File(File&&) noexcept;
  auto operator=(File&&) noexcept -> File&;

  auto as_fd() const -> fd_t;

  operator bool() const;

  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_all(Vec<u8>& buf, usize buf_len = 256) -> Result<usize>;
  auto read_to_string(String& buf, usize buf_len = 256) -> Result<usize>;

  auto write(Slice<const u8> buf) -> Result<usize>;
  auto write_all(Slice<const u8> buf) -> Result<usize>;
  auto write_str(Str str) -> Result<usize>;
};

}  // namespace sfc::io
