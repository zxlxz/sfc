#pragma once

#include "sfc/serde/mod.h"

namespace sfc::serde::json {

auto parse(Str json_str) -> Option<Node>;

auto load(fs::Path path) -> Option<Node>;

}  // namespace sfc::serde::json
