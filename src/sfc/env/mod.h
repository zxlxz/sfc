#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto var(Str key) -> String;
auto set_var(Str key, Str val) -> bool;
auto remove_var(Str key) -> bool;

auto home_dir() -> fs::Path;
auto temp_dir() -> fs::Path;

auto current_exe() -> fs::Path;
auto current_dir() -> fs::Path;
auto set_current_dir(const fs::Path& path) -> bool;

}  // namespace sfc::env
