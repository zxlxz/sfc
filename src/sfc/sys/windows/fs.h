#pragma once

#include "sfc/sys/windows/io.h"

namespace sfc::fs {
struct Metadata;
struct OpenOptions;
}  // namespace sfc::fs

namespace sfc::sys::windows {

struct FileAttr {
  u32 _attr;

 public:
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_link() const noexcept -> bool;
};

struct DirEntry {
  const char* _name = nullptr;
  u64 _size = 0;
  u32 _attr = 0;
};

struct ReadDir {
  void* _handle = nullptr;
  u64 _index = 0;
  u64 _file_size = 0;
  u32 _file_attr = 0;
  char _file_name[256];

 public:
  static auto open(Str path) -> io::Result<ReadDir>;

 public:
  auto close() -> io::Result<>;
  auto next() -> Option<DirEntry>;
};

auto open(Str path, fs::OpenOptions opts) -> io::Result<RawFd>;

auto fstat(RawFd fd) -> io::Result<fs::Metadata>;
auto lstat(Str path) -> io::Result<fs::Metadata>;

auto unlink(Str path) -> io::Result<>;
auto rename(Str old_path, Str new_path) -> io::Result<>;
auto mkdir(Str path) -> io::Result<>;
auto rmdir(Str path) -> io::Result<>;

}  // namespace sfc::sys::windows
