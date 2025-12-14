#pragma once

#include "sfc/io/read.h"
#include "sfc/io/write.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class [[nodiscard]] File {
  fd_t _fd;

 public:
  explicit File() noexcept;
  explicit File(fd_t fd) noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  auto as_fd() const noexcept -> fd_t;
  auto is_open() const noexcept -> bool;

  auto read(Slice<u8> buf) noexcept -> Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> Result<usize>;
};

}  // namespace sfc::io
