#pragma once

#include "path.h"

namespace sfc::fs {

struct Path;

struct Meta {
  u64 _len;
  u32 _mod;

 public:
  auto exists() const -> bool;

  auto file_len() const -> u64;

  auto is_dir() const -> bool;
  auto is_file() const -> bool;
  auto is_link() const -> bool;
};

auto meta(Path path) -> Meta;

}  // namespace sfc::fs
