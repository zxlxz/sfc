#pragma once

#include "sfc/core.h"
#include "sfc/ffi/os_str.h"

namespace sfc::fs {
struct Metadata;
struct OpenOptions;
}  // namespace sfc::fs

namespace sfc::sys::windows {

using RawFd = void*;

struct FileAttr {
  u32 _attr;

 public:
  auto exists() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_link() const noexcept -> bool;
};

struct DirEntry {
  wchar_t _path[260] = {};
  u32 _attr = 0;
  usize _size = 0;
};

struct ReadDir {
  void* handle = nullptr;
  DirEntry entry = {{}, {}};
};

auto open(const wchar_t* path, fs::OpenOptions opts) -> io::Result<RawFd>;

auto fstat(void* handle) -> io::Result<fs::Metadata>;
auto lstat(const wchar_t* path) -> io::Result<fs::Metadata>;

auto unlink(const wchar_t* path) -> io::Result<>;
auto rename(const wchar_t* old_path, const wchar_t* new_path) -> io::Result<>;
auto mkdir(const wchar_t* path) -> io::Result<>;
auto rmdir(const wchar_t* path) -> io::Result<>;

auto open_dir(const wchar_t* path) -> io::Result<ReadDir*>;
auto close_dir(ReadDir* dir) -> io::Result<>;
auto read_dir(ReadDir* dir) -> Option<DirEntry>;

}  // namespace sfc::sys::windows
