#pragma once

#include "sfc/alloc.h"

namespace sfc::env {

auto var(Str key) -> String;
auto set_var(Str key, Str val) -> bool;
auto remove_var(Str key) -> bool;

auto home_dir() -> String;

auto current_dir() -> String;
auto set_current_dir(Str path) -> bool;

}  // namespace sfc::env
