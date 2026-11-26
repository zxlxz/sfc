#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

struct File {
  RawFd _fd = kInvalidFd;

 public:
  explicit File(RawFd fd = kInvalidFd) noexcept;
  ~File() noexcept;
  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;

 public:
  auto close() noexcept -> io::Result<>;
  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(SeekFrom pos) noexcept -> io::Result<u64>;

 public:
  auto is_terminal() const noexcept -> bool;
};

}  // namespace sfc::io
