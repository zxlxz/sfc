#pragma once

#include "sfc/io/mod.h"

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
  auto as_raw_fd() const -> int;
  auto is_valid() const noexcept -> bool;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(io::SeekFrom pos) noexcept -> io::Result<u64>;
};

struct StdIn : io::Read {
  auto is_console() -> bool;
  auto read(Slice<u8> buf) -> io::Result<usize>;
};

struct Stdout : io::Write {
  auto is_console() -> bool;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto flush() -> io::Result<>;
};

struct Stderr : io::Write {
  auto is_console() -> bool;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto flush() -> io::Result<>;
};

auto os_error() -> int;
auto io_error(int code) -> io::Error;

}  // namespace sfc::sys::posix
