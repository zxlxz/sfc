#pragma once

#include "sfc/io/mod.h"
#include "sfc/io/file.h"

namespace sfc::sys::windows {

using io::RawFd;
using io::File;

constexpr u32 kMaxPath = 1024U;

auto close(RawFd fd) -> io::Result<>;
auto read(RawFd fd, Slice<u8> buf) -> io::Result<usize>;
auto write(RawFd fd, Slice<const u8> buf) -> io::Result<usize>;
auto flush(RawFd fd) -> io::Result<>;
auto seek(RawFd fd, io::SeekFrom pos) -> io::Result<u64>;
auto is_console(RawFd fd) -> bool;

auto Stdin() -> File;
auto Stdout() -> File;
auto Stderr() -> File;

auto os_error() -> u32;
auto io_error(u32 code) -> io::Error;

}  // namespace sfc::sys::windows
