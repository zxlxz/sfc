
#include "base64.h"

namespace sfc::serde::base64 {

namespace imp {

static constexpr u8 kENCODE[] = {                                     //
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',  //
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  //
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',  //
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',  //
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static constexpr u8 kDECODE[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   // 000
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   // 010
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   // 020
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   // 030
    0,  0,  0,  62, 0,  0,  0,  63, 52, 53,  // 040
    54, 55, 56, 57, 58, 59, 60, 61, 0,  0,   // 050
    0,  0,  0,  0,  0,  0,  1,  2,  3,  4,   // 060
    5,  6,  7,  8,  9,  10, 11, 12, 13, 14,  // 070
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // 080
    25, 0,  0,  0,  0,  0,  0,  26, 27, 28,  // 090
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110
    49, 50, 51, 0,  0,  0,  0,  0            // 120
};

static auto encode_len(Slice<const u8> buf) -> usize {
  if (buf.is_empty()) {
    return 0U;
  }

  const auto n = buf.len();
  const auto m = n / 3 * 4;
  const auto t = n % 3;
  return m + (t == 0 ? 0 : t + 1);
}

static auto decode_len(Slice<const u8> buf) -> usize {
  // trim
  while (buf._len != 0 && buf[buf._len - 1] == '=') {
    buf._len -= 1;
  }

  if (buf.is_empty()) {
    return 0U;
  }

  const auto n = buf.len();
  const auto m = n / 4 * 3;
  const auto t = n % 4;
  return m + (t <= 1 ? 0 : t - 1);
}

static auto encode(Slice<const u8> src, Slice<u8> dst) -> usize {
  auto f = [](const u8 src[3], u8 dst[4]) {
    dst[0] = kENCODE[src[0] >> 2];
    dst[1] = kENCODE[((src[0] & 0x03) << 4) + (src[1] >> 4)];
    dst[2] = kENCODE[((src[1] & 0x0F) << 2) + (src[2] >> 6)];
    dst[3] = kENCODE[src[2] & 0x3F];
  };

  const auto src_len = src.len();
  const auto dst_len = encode_len(src);
  if (dst_len != dst.len() || dst_len == 0) {
    return dst_len;
  }

  const auto end = src.as_ptr() + src_len;
  auto px = src.as_ptr();
  auto py = dst.as_mut_ptr();
  for (; px + 3 <= end; px += 3, py += 4) {
    f(px, py);
  }

  if (px != end) {
    u8 xx[3] = {0, 0, 0};
    u8 yy[4] = {0, 0, 0, 0};

    const auto nn = static_cast<usize>(end - px);
    ptr::copy_nonoverlapping(px, xx, nn);
    f(xx, yy);
    ptr::copy_nonoverlapping(yy, py, nn + 1);
  }

  return 0;
}

static auto decode(Slice<const u8> src, Slice<u8> dst) -> usize {
  auto f = [](const u8 src[4], u8 dst[3]) {
    const u8 num[4] = {kDECODE[src[0]], kDECODE[src[1]], kDECODE[src[2]], kDECODE[src[3]]};
    dst[0] = static_cast<u8>((num[0] << 2) + (num[1] >> 4));
    dst[1] = static_cast<u8>((num[1] << 4) + (num[2] >> 2));
    dst[2] = static_cast<u8>((num[2] << 6) + num[3]);
  };

  const auto src_len = src.len();
  const auto dst_len = decode_len(src);
  if (dst_len != dst.len() || dst_len == 0) {
    return dst_len;
  }

  const auto end = src.as_ptr() + src_len;
  auto px = src.as_ptr();
  auto py = dst.as_mut_ptr();
  for (; px + 4 <= end; px += 4, py += 3) {
    f(px, py);
  }

  if (px != end) {
    u8 xx[4] = {0, 0, 0, 0};
    u8 yy[3] = {0, 0, 0};

    const auto nn = static_cast<usize>(end - px);
    ptr::copy_nonoverlapping(px, xx, nn);
    f(xx, yy);
    ptr::copy_nonoverlapping(yy, py, nn - 1);
  }

  return 0;
}

}  // namespace imp

auto encode(Slice<const u8> src, Slice<u8> dst) -> usize {
  return imp::encode(src, dst);
}

auto decode(Slice<const u8> src, Slice<u8> dst) -> usize {
  return imp::decode(src, dst);
}

auto encode_vec(Slice<const u8> src) -> Vec<u8> {
  const auto dst_len = imp::encode_len(src);

  auto dst = Vec<u8>::with_capacity(dst_len);
  dst.set_len(dst_len);
  imp::encode(src, dst.as_mut_slice());
  return dst;
}

auto decode_vec(Slice<const u8> src) -> Vec<u8> {
  const auto dst_len = imp::decode_len(src);

  auto dst = Vec<u8>::with_capacity(dst_len);
  dst.set_len(dst_len);
  imp::decode(src, dst.as_mut_slice());
  return dst;
}

}  // namespace sfc::serde::base64
