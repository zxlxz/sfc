#pragma once

#include "sfc/alloc.h"

namespace sfc::serde::base64 {

auto encode(Slice<const u8> src, Slice<u8> dst) -> usize;
auto decode(Slice<const u8> src, Slice<u8> dst) -> usize;

auto encode_vec(Slice<const u8> src) -> Vec<u8>;
auto decode_vec(Slice<const u8> src) -> Vec<u8>;

}  // namespace sfc::serde::base64
