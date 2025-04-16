#pragma once

#include "sfc/io.h"

namespace sfc::fs {

class Path {
  String _inn = {};

 public:
  explicit Path();
  ~Path();
  Path(Path&&) noexcept;
  Path& operator=(Path&&) noexcept;

  static auto from(Str s) -> Path;

  auto clone() const -> Path;

  auto as_str() const -> Str;

  auto c_str() const -> cstr_t;

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

}  // namespace sfc::fs
