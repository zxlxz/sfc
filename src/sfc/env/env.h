#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto get(Str key) -> Str;
void set(Str key, Str val);

auto current_dir() -> fs::Path;
void set_current_dir(const fs::Path& path);

auto current_exe() -> fs::Path;

auto home_dir() -> fs::Path;

}  // namespace sfc::env
