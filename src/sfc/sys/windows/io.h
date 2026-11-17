#pragma once

#include "sfc/io/mod.h"

namespace sfc::sys::windows {

using RawFd = void*;
static constexpr RawFd kInvalidFd = nullptr;

struct File {
  RawFd _fd = kInvalidFd;

 public:
  void close();

  auto read(Slice<u8> buf) -> io::Result<usize>;
  auto write(Slice<const u8> buf) -> io::Result<usize>;
  auto flush() -> io::Result<>;
  auto seek(io::SeekFrom pos) -> io::Result<u64>;

 public:
  auto is_console() const -> bool;
};

auto Stdin() -> File&;
auto Stdout() -> File&;
auto Stderr() -> File&;

auto os_error() -> u32;
auto io_error(u32 code) -> io::Error;

}  // namespace sfc::sys::windows
