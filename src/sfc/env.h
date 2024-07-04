#pragma once

#include "sfc/fs.h"

namespace sfc::env {

auto var(Str key) -> String;
auto set_var(Str key, Str val) -> bool;
auto remove_var(Str key) -> bool;

auto current_dir() -> Str;
auto current_exe() -> Str;
auto home_dir() -> Str;

auto set_current_dir(Str path) -> bool;

}  // namespace sfc::env
