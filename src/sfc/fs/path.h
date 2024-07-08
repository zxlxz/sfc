#pragma once

#include "sfc/ffi.h"

namespace sfc::fs {

struct Path {
  Str _inn;

 public:
  Path() = default;

  Path(const auto& s) : _inn{s} {}

  auto as_str() const -> Str {
    return _inn;
  }

 public:
  // a/b.txt -> b.txt
  auto file_name() const -> Str;

  // a/b.txt -> b
  auto file_stem() const -> Str;

  // a/b.txt -> .txt
  auto extension() const -> Str;

  // a/b.txt -> a
  auto parent() const -> Path;

  // abs path
  auto is_absolute() const -> bool;

  // not absolute
  auto is_relative() const -> bool;

  // 'a'.join('b) -> 'a/b'
  auto join(const auto& p) const -> class PathBuf;

  // 'a'/'b -> 'a/b'
  auto operator/(const auto& p) const -> class PathBuf;

  // fmt
  void fmt(auto& f) const {
    f.pad(_inn);
  }
};

}  // namespace sfc::fs
