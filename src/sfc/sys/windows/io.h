#pragma once

#include "sfc/io/mod.h"
#include "sfc/io/file.h"

namespace sfc::sys::windows {

using RawFd = void*;
static constexpr RawFd kInvalidFd = nullptr;

using File = io::File;

struct StdIo {
  File& _file;

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
