#pragma once

#include "sfc/io.h"

namespace sfc::fs {

struct Path {
  Str _inn = {};

 public:
  auto as_str() const noexcept -> Str;

  auto file_name() const noexcept -> Str;
  auto file_stem() const noexcept -> Str;
  auto extension() const noexcept -> Str;
  auto parent() const noexcept -> Path;
  auto join(Str path) const noexcept -> class PathBuf;

 public:
  auto exists() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;

  auto is_absolute() const noexcept -> bool;
  auto is_relative() const noexcept -> bool;

 public:
  void fmt(auto& f) const {
    _inn.fmt(f);
  }
};

class PathBuf {
  String _inn = {};

 public:
  static auto from(Str s) -> PathBuf;

  auto as_path() const noexcept -> Path;
  auto as_str() const noexcept -> Str;

  void clear();
  void reserve(usize additional);

  void push(Str path) noexcept;
  auto pop() noexcept -> bool;

  void set_file_name(Str file_name) noexcept;
  void set_extension(Str extension) noexcept;

 public:
  void fmt(auto& f) const {
    _inn.fmt(f);
  }
};

struct Meta {
  u32 _attr = 0;
  u64 _size = 0;

 public:
  auto exists() const noexcept -> bool;
  auto file_len() const noexcept -> u64;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
};

auto meta(Path path) -> io::Result<Meta>;

auto create_dir(Path path) -> io::Result<>;

auto remove_dir(Path path) -> io::Result<>;

auto remove_file(Path path) -> io::Result<>;

auto rename(Path from, Path to) -> io::Result<>;

}  // namespace sfc::fs
