#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto var(Str key) -> Str;
void set_var(Str key, Str val);
void remove_var(Str key);

auto current_dir() -> fs::PathBuf;
void set_current_dir(fs::Path path);

auto current_exe() -> fs::PathBuf;

auto home_dir() -> fs::PathBuf;

}  // namespace sfc::env
