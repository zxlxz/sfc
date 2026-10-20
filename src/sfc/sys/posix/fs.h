#pragma once

#include <dirent.h>

#include "sfc/core.h"
#include "sfc/ffi/os_str.h"

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
  char path[1024] = {};
  u32 attr = 0;
  usize size = 0;
};

struct ReadDir {
  DIR* _dir = nullptr;

 public:
  static auto open(const char* path) -> io::Result<ReadDir>;

 public:
  auto close() -> io::Result<>;
  auto next() -> Option<DirEntry>;
};

auto open(const char* path, fs::OpenOptions opts) -> io::Result<RawFd>;

auto fstat(int fd) -> io::Result<fs::Metadata>;
auto lstat(const char* path) -> io::Result<fs::Metadata>;

auto unlink(const char* path) -> io::Result<>;
auto rename(const char* old_path, const char* new_path) -> io::Result<>;
auto mkdir(const char* path) -> io::Result<>;
auto rmdir(const char* path) -> io::Result<>;

}  // namespace sfc::sys::posix
