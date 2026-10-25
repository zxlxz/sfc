#pragma once

#include "sfc/io/mod.h"

namespace sfc::sys::posix {

using RawFd = int;
static constexpr RawFd kInvalidFd = -1;

struct File {
  RawFd _fd = kInvalidFd;

 public:
  void close();

  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto flush() -> io::Result<>;
  auto seek(io::SeekFrom pos) -> io::Result<u64>;
};

struct StdIo {
  File _file;

 public:
  static auto Input() noexcept -> StdIo;
  static auto Output() noexcept -> StdIo;
  static auto Error() noexcept -> StdIo;

 public:
  auto is_console() -> bool;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
};

auto os_error() -> int;
auto io_error(int code) -> io::Error;

}  // namespace sfc::sys::posix
