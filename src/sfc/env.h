#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto var(Str key) -> String;
auto set_var(Str key, Str val) -> bool;
auto remove_var(Str key) -> bool;

auto home_dir() -> fs::PathBuf;
auto temp_dir() -> fs::PathBuf;

auto current_exe() -> fs::PathBuf;
auto current_dir() -> fs::PathBuf;
auto set_current_dir(fs::Path path) -> bool;

}  // namespace sfc::env
