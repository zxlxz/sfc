#include "sfc/serde/base64.h"

namespace sfc::serde::base64 {

static constexpr auto DATA_BUF_LEN = 3U;
static constexpr auto TEXT_BUF_LEN = 4U;
using data_buf_t = u8[DATA_BUF_LEN];
using text_buf_t = char[TEXT_BUF_LEN];

static auto encode_blk(text_buf_t& text, Slice<const u8> data) -> Str {
  static constexpr auto ENCODE_TBL =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

  const auto n = data._len;
  if (n <= 0 || n > DATA_BUF_LEN) {
    return {};
  }

  u32 val = 0;
  for (u32 i = 0; i < DATA_BUF_LEN; ++i) {
    val <<= 8;
    if (i < n) {
      val |= data[i];
    }
  }

  text[0] = ENCODE_TBL[(val >> 18) & 0x3F];
  text[1] = ENCODE_TBL[(val >> 12) & 0x3F];
  text[2] = (n > 1) ? ENCODE_TBL[(val >> 6) & 0x3F] : '=';
  text[3] = (n > 2) ? ENCODE_TBL[val & 0x3F] : '=';
  return Str{text, 4};
}

// @note: in won't contains '=' char
static auto decode_blk(data_buf_t& data, Str text) -> Slice<const u8> {
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

  const auto n = text._len;
  if (n <= 0 || n > TEXT_BUF_LEN) {
    return {};
  }

  auto val = 0U;
  for (auto i = 0U; i < TEXT_BUF_LEN; ++i) {
    val <<= 6;
    if (i < n) {
      const auto c = i32{text[i]};
      const auto u = c < i32{sizeof(DECODE_MAP)} ? DECODE_MAP[c] : FF;
      if (u == FF) {
        return {};
      }
      val |= u;
    }
  }

  if (n > 1) data[0] = (val >> 16) & 0xFF;
  if (n > 2) data[1] = (val >> 8) & 0xFF;
  if (n > 3) data[2] = val & 0xFF;
  return Slice<const u8>{data, n - 1};
}

auto encode(Slice<const u8> data) -> String {
  if (data.is_empty()) {
    return {};
  }

  const auto data_len = data.len();
  const auto text_len = ((data_len + DATA_BUF_LEN - 1) / DATA_BUF_LEN) * TEXT_BUF_LEN;

  auto text = String{};
  text.reserve(text_len);

  for (auto i = 0u; i < data_len; i += DATA_BUF_LEN) {
    const auto is_end = i + DATA_BUF_LEN >= data_len;
    const auto in_ptr = data._ptr + i;
    const auto in_len = is_end ? data_len - i : DATA_BUF_LEN;

    text_buf_t out_buf = {};
    const auto out_str = base64::encode_blk(out_buf, Slice{in_ptr, in_len});
    text.push_str(out_str);
  }

  return text;
}

auto decode(Str text) -> List<u8> {
  if (text.is_empty()) {
    return List<u8>{};
  }

  const auto text_len = text.len();
  if (text_len % 4 != 0) {
    return List<u8>{};
  }

  auto out = List<u8>{};
  out.reserve((text_len / TEXT_BUF_LEN) * DATA_BUF_LEN);

  for (auto i = 0u; i < text._len; i += TEXT_BUF_LEN) {
    const auto is_end = i + 4 >= text._len;
    const auto in_ptr = text._ptr + i;
    const auto in_len = is_end ? (in_ptr[2] == '=' ? 2U : (in_ptr[3] == '=' ? 3U : 4U)) : TEXT_BUF_LEN;

    data_buf_t out_buf = {};
    const auto out_data = base64::decode_blk(out_buf, Str{in_ptr, in_len});
    if (out_data.is_empty()) {
      break;
    }
    out.extend_from_slice(out_data);
  }

  return out;
}

}  // namespace sfc::serde::base64
