#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class File : public io::Read, public io::Write {
  fd_t _raw;

 public:
  explicit File() noexcept;
  explicit File(fd_t fd) noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

 public:
  auto as_raw_fd() const noexcept -> fd_t;

  auto read(Slice<u8> buf) noexcept -> Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> Result<usize>;
};

auto last_os_error() noexcept -> Error;

}  // namespace sfc::io
