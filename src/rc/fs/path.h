#pragma once

#include "rc/alloc.h"

namespace rc::fs {

struct Path {
  Str _inner;

  Path(const Str& s) noexcept: _inner{s}{}
  operator Str() const noexcept { return _inner; }

  pub auto parent() const -> Path;
  pub auto file_name() const -> Str;
  pub auto file_stem() const -> Str;
  pub auto extension() const -> Str;
  pub auto exists() const -> bool;
  pub auto is_file() const -> bool;
  pub auto is_dir() const -> bool;
};

struct PathBuf {
  string::String _inner;

  pub explicit PathBuf(Str s);

  pub auto as_str() const noexcept -> Str;
  pub auto as_path() const noexcept -> Path;
  pub auto clone() const -> PathBuf;
  pub auto exists() const -> bool;
  pub auto is_file() const -> bool;
  pub auto is_dir() const -> bool;

  pub auto parent() const -> Path;
  pub auto file_name() const -> Str;
  pub auto file_stem() const -> Str;
  pub auto extension() const -> Str;

  pub auto push(Str s) -> void;
  pub auto pop() -> void;
  pub auto set_file_name(Str new_name) -> void;
  pub auto set_extension(Str new_ext) -> void;
};

pub auto exists(Path p) -> bool;
pub auto is_dir(Path p) -> bool;
pub auto is_file(Path p) -> bool;

pub auto create_dir(Path p) -> void;
pub auto remove_file(Path p) -> void;
pub auto remove_dir(Path p) -> void;

pub auto copy(Path from, Path to) -> void;
pub auto rename(Path from, Path to) -> void;

auto create_dir_all(Path p) -> void = delete;
auto remove_dir_all(Path p) -> void = delete;

}  // namespace rc::fs
