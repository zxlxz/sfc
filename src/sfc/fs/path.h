#pragma once

#include "sfc/core.h"

namespace sfc::fs {

struct Path {
  Str _inn = {};

 public:
  Path(const auto& s) noexcept : _inn{s} {}

  auto as_str() const noexcept -> Str;

  auto file_name() const noexcept -> Str;
  auto file_stem() const noexcept -> Str;
  auto extension() const noexcept -> Str;
  auto parent() const noexcept -> Path;

 public:
  auto exists() const noexcept -> bool;
  auto is_file() const noexcept -> bool;
  auto is_dir() const noexcept -> bool;

  auto is_root() const noexcept -> bool;
  auto is_absolute() const noexcept -> bool;
  auto is_relative() const noexcept -> bool;

 public:
  // trait: ops::Eq
  auto operator==(const Path& p) const noexcept -> bool;

  // trait: fmt::Display
  void fmt(fmt::Formatter& f) const;
};

}  // namespace sfc::fs
