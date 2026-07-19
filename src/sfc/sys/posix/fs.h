#pragma once

#include "sfc/core.h"

namespace sfc::fs {
struct Metadata;
struct OpenOptions;
}  // namespace sfc::fs

namespace sfc::sys::posix {

using RawFd = int;

struct FileAttr {
  u32 _attr;

 public:
  auto is_dir() const -> bool;
  auto is_file() const -> bool;
};

auto open(const char* path, fs::OpenOptions opts) -> io::Result<RawFd>;

auto lstat(const char* path) -> io::Result<fs::Metadata>;
auto unlink(const char* path) -> io::Result<>;
auto rename(const char* old_path, const char* new_path) -> io::Result<>;
auto mkdir(const char* path) -> io::Result<>;
auto rmdir(const char* path) -> io::Result<>;

}  // namespace sfc::sys::posix
