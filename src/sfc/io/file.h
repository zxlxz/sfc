#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

#ifdef _WIN32
using RawFd = void*;
#else
using RawFd = int;
#endif

class [[nodiscard]] File : public io::Read, public io::Write {
  RawFd _fd;

 public:
  explicit File(RawFd fd) noexcept;
  ~File() noexcept;
  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(RawFd fd) noexcept -> File;

 public:
  auto as_raw_fd() const noexcept -> RawFd;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(SeekFrom pos) noexcept -> io::Result<u64>;
};

}  // namespace sfc::io
