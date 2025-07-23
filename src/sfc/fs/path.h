#pragma once

#include "sfc/io.h"

namespace sfc::fs {

class Path {
  String _inn = {};

 public:
  explicit Path();

  ~Path();

  Path(Path&&) noexcept;

  auto operator=(Path&&) noexcept -> Path&;

  static auto from(Str s) -> Path;

  auto clone() const -> Path;

  auto as_ptr() const -> const char*;

  auto as_str() const -> Str;

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

  // 'a'.push('b') -> 'a/b'
  void push(Str path);

  // 'a/b'.pop() -> 'a'
  auto pop() -> bool;

  // 'a'.join('b) -> 'a/b'
  auto join(Str path) const -> Path;

  void set_file_name(Str file_name);

  void set_extension(Str extension);

  void fmt(auto& f) const {
    f.pad(_inn.as_str());
  }

 public:
  auto exists() const -> bool;

  auto is_file() const -> bool;

  auto is_dir() const -> bool;
};

struct Meta {
  u32 _attr = 0;
  u64 _size = 0;

 public:
  auto exists() const -> bool;

  auto file_len() const -> u64;

  auto is_dir() const -> bool;
  auto is_file() const -> bool;
};

auto meta(const Path& path) -> io::Result<Meta>;

auto create_dir(const Path& path) -> io::Result<>;

auto remove_dir(const Path& path) -> io::Result<>;

auto remove_file(const Path& path) -> io::Result<>;

auto rename(const Path& from, const Path& to) -> io::Result<>;

}  // namespace sfc::fs
