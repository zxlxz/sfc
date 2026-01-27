#pragma once

#include "sfc/io/read.h"
#include "sfc/io/write.h"

namespace sfc::io {

#ifdef _WIN32
using fd_t = void*;
#else
using fd_t = int;
#endif

class File : public io::Read, public io::Write {
 public:
  fd_t _fd;

 public:
  explicit File() noexcept;
  explicit File(fd_t fd) noexcept;
  ~File() noexcept;

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  auto read(Slice<u8> buf) noexcept -> Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> Result<usize>;
};

auto last_os_error() noexcept -> Error;

}  // namespace sfc::io
