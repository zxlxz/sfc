#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto var(Str key) -> String;

void set_var(Str key, Str val);

void remove_var(Str key);

auto current_dir() -> fs::Path;

void set_current_dir(const fs::Path& path);

auto current_exe() -> fs::Path;

auto home_dir() -> fs::Path;

}  // namespace sfc::env
