#pragma once

#include "sfc/io.h"

namespace sfc::fs {

class PathBuf;

struct Path {
  Str _inn = {};

 public:
  Path(const auto& s) noexcept : _inn{s} {}

  auto as_str() const noexcept -> Str;

  auto file_name() const noexcept -> Str;
  auto file_stem() const noexcept -> Str;
  auto extension() const noexcept -> Str;
  auto parent() const noexcept -> Path;

  auto join(Str path) const noexcept -> PathBuf;

 public:
  auto exists() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;

  auto is_root() const noexcept -> bool;
  auto is_absolute() const noexcept -> bool;
  auto is_relative() const noexcept -> bool;

 public:
  // trait: ops::Eq
  auto operator==(const Path& p) const -> bool {
    return _inn == p._inn;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    _inn.fmt(f);
  }
};

class PathBuf {
  String _inn = {};

 public:
  static auto from(Str s) noexcept -> PathBuf;

  auto as_path() const noexcept -> Path;
  auto as_str() const noexcept -> Str;

 public:
  auto clone() const -> PathBuf;
  void clear() noexcept;
  void reserve(usize additional) noexcept;

  void push(Path path) noexcept;
  auto pop() noexcept -> bool;

  void set_file_name(Str file_name) noexcept;
  void set_extension(Str extension) noexcept;

  auto join(Path path) const -> PathBuf;

 public:
  // trait: deref<Path>
  auto operator*() const noexcept -> Path {
    return this->as_path();
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    _inn.fmt(f);
  }
};

struct Metadata {
  u32 _attr = 0;
  u64 _size = 0;

 public:
  auto exists() const noexcept -> bool;
  auto file_len() const noexcept -> u64;
  auto is_dir() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
};

auto metadata(Path path) -> io::Result<Metadata>;

auto create_dir(Path path) -> io::Result<>;
auto create_dir_all(Path path) -> io::Result<>;

auto remove_dir(Path path) -> io::Result<>;
auto remove_file(Path path) -> io::Result<>;

auto rename(Path from, Path to) -> io::Result<>;

}  // namespace sfc::fs
