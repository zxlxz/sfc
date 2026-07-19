#pragma once

#include "sfc/core.h"

namespace sfc::fs {
struct Metadata;
struct OpenOptions;
}  // namespace sfc::fs

namespace sfc::sys::windows {

using RawFd = void*;

struct FileAttr {
  u32 _attr;

 public:
  auto is_dir() const -> bool;
  auto is_file() const -> bool;
};

using fs::OpenOptions;
using fs::Metadata;

auto open(const wchar_t* path, OpenOptions opts) -> io::Result<RawFd>;

auto lstat(const wchar_t* path) -> io::Result<Metadata>;
auto unlink(const wchar_t* path) -> io::Result<>;
auto rename(const wchar_t* old_path, const wchar_t* new_path) -> io::Result<>;
auto mkdir(const wchar_t* path) -> io::Result<>;
auto rmdir(const wchar_t* path) -> io::Result<>;

}  // namespace sfc::sys::windows
