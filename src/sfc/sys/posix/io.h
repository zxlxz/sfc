#pragma once

#include "sfc/core.h"

namespace sfc::sys::posix {

using RawFd = int;

class File {
  int _fd = -1;

 public:
  File(int fd = -1) noexcept;
  ~File() noexcept;

  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;

 public:
  auto is_valid() const noexcept -> bool;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(i64 offset, int whence) noexcept -> io::Result<u64>;
};

struct StdIn {
  auto is_console() -> bool;
  auto read(Slice<u8> buf) -> io::Result<usize>;
};

struct Stdout {
  auto is_console() -> bool;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
};

struct Stderr {
  auto is_console() -> bool;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
};

auto os_error() -> int;
auto io_error(int code) -> io::Error;

}  // namespace sfc::sys::posix
