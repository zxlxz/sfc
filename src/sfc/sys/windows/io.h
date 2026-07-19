#pragma once

#include "sfc/io/mod.h"

namespace sfc::sys::windows {

using RawFd = void*;

class File {
  void* _fd{nullptr};

 public:
  File(void* fd = nullptr) noexcept;
  ~File() noexcept;

  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;

 public:
  auto is_valid() const noexcept -> bool;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(io::SeekFrom whence) noexcept -> io::Result<u64>;
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

auto os_error() -> u32;
auto io_error(u32 code) -> io::Error;

}  // namespace sfc::sys::windows
