#pragma once

#include "sfc/alloc/string.h"

namespace sfc::serde::base64 {

auto encode(Slice<const u8> data) -> String;
auto decode(Str input) -> Vec<u8>;

}  // namespace sfc::serde::base64
