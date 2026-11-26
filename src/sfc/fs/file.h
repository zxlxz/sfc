#pragma once

#include "sfc/io/file.h"

namespace sfc::fs {

struct Metadata;

class [[nodiscard]] File : public io::IRead, public io::IWrite, public io::ISeek {
  io::File _inn;

  explicit File(io::File file) noexcept;

 public:
  ~File();

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(io::RawFd fd) -> File;
  static auto open(Str path) noexcept -> io::Result<File>;
  static auto create(Str path) noexcept -> io::Result<File>;

 public:
  auto read(Slice<u8> buf) noexcept -> io::Result<usize>;
  auto write(Slice<const u8> buf) noexcept -> io::Result<usize>;
  auto flush() noexcept -> io::Result<>;
  auto seek(io::SeekFrom pos) noexcept -> io::Result<u64>;

  auto metadata() noexcept -> io::Result<Metadata>;
};

struct OpenOptions {
  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  auto open(Str path) const noexcept -> io::Result<File>;
};

struct Metadata {
  u64 _size = 0;
  u32 _attr = 0;

 public:
  auto file_size() const noexcept -> u64;
  auto exists() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_link() const noexcept -> bool;
};

struct DirEntry {
  Str path = {};
  Metadata meta = {};
};

class [[nodiscard]] ReadDir {
  struct Inn;
  Inn* _inn;

 public:
  explicit ReadDir(Inn& inn) noexcept;
  ~ReadDir();

  ReadDir(ReadDir&&) noexcept;
  ReadDir& operator=(ReadDir&&) noexcept;

  static auto open(Str path) -> io::Result<ReadDir>;

 public:
  auto next() -> Option<DirEntry>;
};

auto read(Str path) noexcept -> io::Result<List<u8>>;
auto write(Str path, Slice<const u8> buf) noexcept -> io::Result<>;

auto metadata(Str path) -> io::Result<Metadata>;

auto create_dir(Str path) -> io::Result<>;
auto create_dir_all(Str path) -> io::Result<>;
auto remove_dir(Str path) -> io::Result<>;
auto remove_file(Str path) -> io::Result<>;
auto rename(Str old_path, Str new_path) -> io::Result<>;

auto read_dir(Str path) -> io::Result<ReadDir>;

}  // namespace sfc::fs
