#pragma once

#include "sfc/io/file.h"
#include "sfc/fs/path.h"

namespace sfc::fs {

class File;

struct Metadata {
  u32 _attr = {};
  u64 _size = 0;

 public:
  auto file_len() const noexcept -> u64;

  auto exists() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_link() const noexcept -> bool;
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

class [[nodiscard]] File : public io::File {
 public:
  explicit File(io::File file) noexcept;
  ~File();

  File(File&&) noexcept;
  File& operator=(File&&) noexcept;

  static auto from_raw_fd(io::RawFd fd) -> File;
  static auto open(Path path) noexcept -> io::Result<File>;
  static auto create(Path path) noexcept -> io::Result<File>;

 public:
  auto metadata() noexcept -> io::Result<Metadata>;
};

class [[nodiscard]] ReadDir {
  struct Inn;
  Inn* _inn;

 public:
  explicit ReadDir(Inn& inn) noexcept;
  ~ReadDir();

  ReadDir(ReadDir&&) noexcept;
  ReadDir& operator=(ReadDir&&) noexcept;

  static auto open(Path path) -> io::Result<ReadDir>;

 public:
  struct Entry {
    PathBuf path = {};
    Metadata meta = {};
  };
  auto next() -> Option<Entry>;
};

auto read(Path path) noexcept -> io::Result<List<u8>>;
auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<>;

auto metadata(Path path) -> io::Result<Metadata>;

auto create_dir(Path path) -> io::Result<>;
auto create_dir_all(Path path) -> io::Result<>;
auto remove_dir(Path path) -> io::Result<>;
auto remove_file(Path path) -> io::Result<>;
auto rename(Path from, Path to) -> io::Result<>;

auto read_dir(Path path) -> io::Result<ReadDir>;

}  // namespace sfc::fs
