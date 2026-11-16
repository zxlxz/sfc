#pragma once

#include <dirent.h>
#include "sfc/sys/posix/mod.h"

namespace sfc::fs {
struct Metadata;
struct OpenOptions;
}  // namespace sfc::fs

namespace sfc::sys::posix {

using RawFd = int;

struct FileAttr {
  u32 _attr;

 public:
  auto exists() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_link() const noexcept -> bool;
};

struct DirEntry {
  const char* name;
  u32 attr = 0;
  usize size = 0;
};

struct ReadDir {
  DIR* _dir = nullptr;

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

}  // namespace sfc::sys::posix
