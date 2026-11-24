#include "str_set.h"

namespace sfc::collections {

namespace {

template <class B>
auto sblk_new(Str s) -> B {
  const auto n = s._len < sizeof(B) ? s._len : sizeof(B);

  auto b = B{};
  auto p = (u8*)&b;
  for (auto i = 0U; i < n; ++i) {
    p[i] = s._ptr[i];
  }
  return b;
}

template <class B>
auto sblk_count_bytes(const B& blk) -> u32 {
  const auto p = (const u8*)&blk;

  auto n = 0U;
  for (; n < sizeof(B); ++n) {
    if (p[n] == 0) break;
  }
  return n;
}

template <class B>
auto sblk_find_tail(const B blk[]) -> usize {
  static constexpr u32 N = sizeof(B);

  auto p = blk;
  while (((const u8*)p)[N - 1] != 0) {
    ++p;
  }
  return usize(p - blk);
}

}  // namespace

StrSet::StrSet(List<u64> buf) noexcept : _buf{mem::move(buf)} {}

StrSet::~StrSet() {}

auto StrSet::new_() -> StrSet {
  return StrSet{List<u64>{}};
}

auto StrSet::with_capacity(usize capacity) -> StrSet {
  return StrSet{List<u64>::with_capacity(capacity)};
}

void StrSet::reserve(usize add_cnt) {
  _buf.reserve(add_cnt);
}

auto StrSet::search(Str s) const -> Option<u32> {
  const auto buf_ptr = _buf.as_ptr();
  const auto buf_cnt = _buf.len();
  const auto s0 = sblk_new<u64>(s);

  struct SearchBlock {
    const u64* _buf;
    usize _cnt;

    auto is_equal(u64 b0, Str s) const -> bool {
      if (_buf[0] != b0) return false;       // head not match
      if (s._len < kBlockSize) return true;  // short string matches head block

      const auto blk_cap = _cnt * kBlockSize;
      if (s._len >= blk_cap) return false;  // string too long for this block sequence

      const auto tail_len = sblk_count_bytes(_buf[_cnt - 1]);
      if (s._len != (_cnt - 1) * kBlockSize + tail_len) return false;

      return __builtin_memcmp(_buf + 1, s._ptr + kBlockSize, s._len - kBlockSize) == 0;
    }
  };

  auto bid = 0U;
  while (bid < buf_cnt) {
    const auto blk_ptr = buf_ptr + bid;
    const auto blk_len = 1 + sblk_find_tail(blk_ptr);

    const auto blk = SearchBlock{blk_ptr, blk_len};
    if (blk.is_equal(s0, s)) {
      return u32{bid};
    }
    bid += blk_len;
  }

  return {};
}

auto StrSet::insert(Str s) -> InsertResult {
  const auto search_idx = this->search(s);
  if (search_idx.is_some()) {
    return InsertResult{*search_idx, false};
  }

  const auto [u8_ptr, u8_len] = s.as_bytes();

  // block contains 'str body with tail 0 byte'
  const auto blk_cnt = u8_len / kBlockSize + 1;

  _buf.reserve(blk_cnt);
  const auto [buf_ptr, buf_len] = _buf.as_mut_slice();

  const auto dst_ptr = buf_ptr + buf_len;
  dst_ptr[blk_cnt - 1] = 0;                   // init tail to zero
  __builtin_memcpy(dst_ptr, u8_ptr, u8_len);  // copy content

  _buf.set_len(buf_len + blk_cnt);

  return InsertResult{u32(buf_len), true};
}

}  // namespace sfc::collections
