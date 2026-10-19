#pragma once

#include "sfc/io/mod.h"

namespace sfc::sys::windows {

using RawFd = void*;

struct File {
  void* _fd{nullptr};

 public:
  auto close() -> io::Result<>;
  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto flush() -> io::Result<>;
  auto seek(io::SeekFrom whence) -> io::Result<u64>;
};

struct StdIo : File {
 public:
  static auto Input() -> StdIo;
  static auto Output() -> StdIo;
  static auto Error() -> StdIo;

 public:
  auto is_console() const -> bool;
  auto is_utf8_console() const -> bool;

  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto read_u16(Slice<u16> buf) -> io::Result<usize>;

  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto write_u16(Slice<const u16> buf) -> io::Result<usize>;

  auto flush() -> io::Result<>;
};

auto os_error() -> u32;
auto io_error(u32 code) -> io::Error;

}  // namespace sfc::sys::windows
