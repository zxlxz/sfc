#pragma once

#include "sfc/io/error.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class [[nodiscard]] File {
  fd_t _fd;

 public:
  explicit File(fd_t fd);
  ~File()noexcept;

  File(File&&) noexcept;
  auto operator=(File&&) noexcept -> File&;

  auto as_fd() const -> fd_t;
  auto is_open() const -> bool;

  auto read(Slice<u8> buf) -> Result<usize>;
  auto read_all(Vec<u8>& buf, usize buf_len = 256) -> Result<usize>;

  auto write(Slice<const u8> buf) -> Result<usize>;
  auto write_all(Slice<const u8> buf) -> Result<usize>;
};

}  // namespace sfc::io
