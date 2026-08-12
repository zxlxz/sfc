#pragma once

#include "sfc/io/mod.h"
#include "sfc/fs/path.h"
#include "sfc/sys/io.h"

namespace sfc::fs {

class File;

struct Metadata {
  u32 _attr = 0;
  u64 _size = 0;

 public:
  auto exists() const noexcept -> bool;
  auto file_len() const noexcept -> u64;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
};

class [[nodiscard]] File : public io::Read, public io::Write {
  sys::File _inn;

 public:
  File() noexcept;
  ~File() noexcept;
  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(sys::RawFd fd) -> File;
  static auto open(Path path) noexcept -> io::Result<File>;
  static auto create(Path path) noexcept -> io::Result<File>;

 public:
  auto as_raw_fd() const noexcept -> sys::RawFd;
  auto is_valid() const -> bool;

  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() -> io::Result<>;
  auto seek(io::SeekFrom pos) noexcept -> io::Result<usize>;

  auto metadata() noexcept -> io::Result<fs::Metadata>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(Path path) const noexcept -> io::Result<File>;
};

auto read(Path path) noexcept -> io::Result<List<u8>>;
auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<>;

auto metadata(Path path) -> io::Result<Metadata>;

auto create_dir(Path path) -> io::Result<>;
auto create_dir_all(Path path) -> io::Result<>;
auto remove_dir(Path path) -> io::Result<>;
auto remove_file(Path path) -> io::Result<>;
auto rename(Path from, Path to) -> io::Result<>;

}  // namespace sfc::fs
