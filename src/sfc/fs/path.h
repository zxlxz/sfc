#pragma once

#include "sfc/io.h"

namespace sfc::fs {

class PathBuf;

struct Path {
  Str _inn = {};

 public:
  Path() noexcept = default;

  Path(Str s) noexcept : _inn{s} {}

  auto as_str() const noexcept -> Str {
    return _inn;
  }

  auto operator==(const Path& other) const noexcept -> bool {
    return _inn == other._inn;
  }

  // a/b.txt -> b.txt
  auto file_name() const noexcept -> Str;

  // a/b.txt -> b
  auto file_stem() const noexcept -> Str;

  // a/b.txt -> .txt
  auto extension() const noexcept -> Str;

  // a/b.txt -> a
  auto parent() const noexcept -> Path;

  // 'a'.join('b) -> 'a/b'
  auto join(Str path) const noexcept -> PathBuf;

  // abs path
  auto is_absolute() const noexcept -> bool;

  // not absolute
  auto is_relative() const noexcept -> bool;

 public:
  auto exists() const noexcept -> bool;

  auto is_file() const noexcept -> bool;

  auto is_dir() const noexcept -> bool;

 public:
  void fmt(auto& f) const {
    f.pad(_inn);
  }
};

class PathBuf {
  String _inn = {};

 public:
  PathBuf() noexcept;
  ~PathBuf() noexcept;

  PathBuf(PathBuf&&) noexcept;
  PathBuf& operator=(PathBuf&&) noexcept;

  static auto from(Str s) -> PathBuf;

  auto as_path() const -> Path;

  void clear();

  void reserve(usize additional);

  // 'a'.push('b') -> 'a/b'
  void push(Str path) noexcept;

  // 'a/b'.pop() -> 'a'
  auto pop() noexcept -> bool;

  // 'a/b.txt'.set_file_name('c.txt') -> 'a/c.txt'
  void set_file_name(Str file_name) noexcept;

  // 'a/b.txt'.set_extension('dat') -> 'a/b.dat'
  void set_extension(Str extension) noexcept;

 public:
  void fmt(auto& f) const {
    f.pad(_inn.as_str());
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
