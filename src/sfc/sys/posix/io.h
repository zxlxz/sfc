#pragma once

#include "sfc/io/mod.h"

namespace sfc::sys::posix {

using RawFd = int;

struct File {
  int _fd{-1};

 public:
  auto close() noexcept -> io::Result<>;
  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(io::SeekFrom pos) noexcept -> io::Result<u64>;
};

struct StdIo : File {
 public:
  static auto Input() noexcept -> StdIo;
  static auto Output() noexcept -> StdIo;
  static auto Error() noexcept -> StdIo;

 public:
  auto is_console() -> bool;
};

auto os_error() -> int;
auto io_error(int code) -> io::Error;

}  // namespace sfc::sys::posix
