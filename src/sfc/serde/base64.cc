#include "sfc/serde/base64.h"

namespace sfc::serde::base64 {

static auto encode_blk(const u8 in[3], u8 (&out)[4], usize in_len) -> usize {
  static constexpr auto ENCODE_TBL =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

  if (in_len <= 0 || in_len > 3) {
    return 0;
  }

  u32 val = 0;
  for (u32 i = 0; i < 3; ++i) {
    val <<= 8;
    if (i < in_len) {
      val |= in[i];
    }
  }

  out[0] = ENCODE_TBL[(val >> 18) & 0x3F];
  out[1] = ENCODE_TBL[(val >> 12) & 0x3F];
  out[2] = (in_len > 1) ? ENCODE_TBL[(val >> 6) & 0x3F] : '=';
  out[3] = (in_len > 2) ? ENCODE_TBL[val & 0x3F] : '=';
  return 4;
}

// @note: in won't contains '=' char
static auto decode_blk(const u8 in[4], u8 (&out)[3], usize in_len) -> usize {
  static constexpr u8 FF = 0xFF;

  static constexpr u8 DECODE_MAP[] = {
      FF, FF, FF, FF, FF, FF, FF, FF,  //
      FF, FF, FF, FF, FF, FF, FF, FF,  //
      FF, FF, FF, FF, FF, FF, FF, FF,  //
      FF, FF, FF, FF, FF, FF, FF, FF,  //
      FF, FF, FF, FF, FF, FF, FF, FF,  //
      FF, FF, FF, 62, FF, FF, FF, 63,  // +, /
      52, 53, 54, 55, 56, 57, 58, 59,  // 0 .. 7
      60, 61, FF, FF, FF, FF, FF, FF,  // 8 .. 9
      FF, 0,  1,  2,  3,  4,  5,  6,   // A .. G
      7,  8,  9,  10, 11, 12, 13, 14,  // H .. O
      15, 16, 17, 18, 19, 20, 21, 22,  // P .. W
      23, 24, 25, FF, FF, FF, FF, FF,  // X .. Z
      FF, 26, 27, 28, 29, 30, 31, 32,  // a .. g
      33, 34, 35, 36, 37, 38, 39, 40,  // h .. o
      41, 42, 43, 44, 45, 46, 47, 48,  // p .. w
      49, 50, 51, FF, FF, FF, FF, FF,  // x .. z
  };

  if (in_len <= 0 || in_len > 4) {
    return 0;
  }

  auto val = 0U;
  for (auto i = 0U; i < 4; ++i) {
    val <<= 6;
    if (i < in_len) {
      const auto c = in[i];
      const auto u = c < sizeof(DECODE_MAP) ? DECODE_MAP[c] : FF;
      if (u == FF) {
        return 0;
      }
      val |= u;
    }
  }

  if (in_len > 1) {
    out[0] = (val >> 16) & 0xFF;
  }
  if (in_len > 2) {
    out[1] = (val >> 8) & 0xFF;
  }
  if (in_len > 3) {
    out[2] = val & 0xFF;
  }

  return in_len - 1;
}

auto encode(Slice<const u8> data) -> String {
  if (data.is_empty()) {
    return {};
  }

  const auto data_len = data.len();
  const auto text_len = ((data_len + 2) / 3) * 4;

  auto text = String{};
  text.reserve(text_len);

  auto& text_buf = text.as_mut_vec();
  for (auto i = 0u; i < data_len; i += 3) {
    const auto in_blk = data._ptr + i;
    const auto in_len = i + 3 <= data_len ? 3 : data_len - i;

    u8 out_blk[4] = {};
    encode_blk(in_blk, out_blk, in_len);

    text_buf.extend_from_slice({out_blk, 4});
  }

  return text;
}

auto decode(Str text) -> Vec<u8> {
  if (text.is_empty()) {
    return {};
  }

  const auto text_len = text.len();
  if (text_len % 4 != 0) {
    return {};
  }

  auto out = Vec<u8>{};
  out.reserve((text_len / 4) * 3);

  const auto in_ptr = text.as_bytes().as_ptr();
  for (auto i = 0u; i < text._len; i += 4) {
    const auto in_blk = in_ptr + i;
    const auto is_end = i + 4 >= text._len;
    const auto in_len = is_end ? (in_blk[2] == '=' ? 2U : (in_blk[3] == '=' ? 3U : 4U)) : 4U;

    u8 out_blk[3] = {};
    const auto out_len = decode_blk(in_blk, out_blk, in_len);
    if (out_len == 0) {
      return {};
    }
    out.extend_from_slice({out_blk, out_len});
  }

  return out;
}

}  // namespace sfc::serde::base64
