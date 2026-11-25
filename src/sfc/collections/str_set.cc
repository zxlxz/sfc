#include "str_set.h"

namespace sfc::collections {

namespace {

auto sblk_find_tail(const auto* p) noexcept -> u32 {
  auto n = 0U;
  while (!p[n].is_tail()) {
    ++n;
  }
  return n;
}

template <class B>
auto sblk_equal(Slice<const B> blks, const B& b0, Str s) noexcept -> bool {
  const auto [v, n] = blks;

  // 1. fast check based on string length and block size
  if (n == 0) return false;
  if (s._len < (n - 1) * B::kLEN) return false;
  if (s._len >= (n + 1) * B::kLEN) return false;

  // 2. check first block
  if (v[0] != b0) return false;
  if (s._len < B::kLEN) return true;

  // 3. calculate total block bytes count, and check against string length
  const auto tail_len = v[n - 1].len();
  const auto blk_bytes_cnt = (n - 1) * B::kLEN + tail_len;
  if (s._len != blk_bytes_cnt) return false;

  // 4. compare remaining blocks with string tail
  const auto v_tail = v + 1;
  const auto s_tail = s._ptr + sizeof(B);
  return __builtin_memcmp(v_tail, s_tail, s._len - B::kLEN) == 0;
}

}  // namespace

struct StrSet::V16B {
  static constexpr u32 kLEN = 16;
  union {
    __int128 _u;
    char data[kLEN];
  };

 public:
  static auto from_str(Str s) -> V16B {
    const auto n = s._len < kLEN ? s._len : kLEN;

    auto blk = V16B{{0}};
    for (auto i = 0U; i < n; ++i) {
      blk.data[i] = s._ptr[i];
    }
    return blk;
  }

 public:
  auto is_tail() const noexcept -> bool {
    return data[kLEN - 1] == 0;
  }

  auto len() const -> u32 {
    auto n = kLEN;
    while (n > 0 && data[n - 1] == 0) {
      --n;
    }
    return n;
  }

  auto operator==(const V16B& other) const noexcept -> bool {
    return other._u == _u;
  }
};

StrSet::StrSet(List<V16B> buf) noexcept : _buf{mem::move(buf)} {}

StrSet::~StrSet() = default;

StrSet::StrSet(StrSet&&) noexcept = default;

StrSet& StrSet::operator=(StrSet&&) noexcept = default;

auto StrSet::new_() -> StrSet {
  return StrSet{List<V16B>{}};
}

auto StrSet::with_capacity(usize capacity) -> StrSet {
  return StrSet{List<V16B>::with_capacity(capacity)};
}

void StrSet::reserve(usize add_cnt) {
  _buf.reserve(add_cnt);
}

auto StrSet::search(Str s) const -> Option<u32> {
  const auto [buf_ptr, buf_cnt] = _buf.as_slice();
  const auto b0 = V16B::from_str(s);

  auto bid = 0U;
  while (bid < buf_cnt) {
    const auto blk_ptr = buf_ptr + bid;
    const auto blk_len = 1 + sblk_find_tail(blk_ptr);

    if (sblk_equal({blk_ptr, blk_len}, b0, s)) {
      return u32{bid};
    }

    bid += blk_len;
  }

  return {};
}

auto StrSet::insert(Str s) -> InsertResult {
  static constexpr usize kLEN = V16B::kLEN;

  const auto search_idx = this->search(s);
  if (search_idx.is_some()) {
    return InsertResult{*search_idx, false};
  }

  const auto old_len = _buf.len();

  // block contains 'str body with tail 0 byte'
  const auto blk_cnt = s._len / kLEN + 1;
  const auto head_len = (blk_cnt - 1) * kLEN;
  const auto tail_blk = V16B::from_str({s._ptr + head_len, s._len - head_len});

  _buf.reserve(blk_cnt);
  const auto dst_ptr = _buf.as_mut_ptr() + old_len;
  __builtin_memcpy(dst_ptr, s._ptr, head_len);
  __builtin_memcpy(dst_ptr + (blk_cnt - 1), &tail_blk, kLEN);
  _buf.set_len(old_len + blk_cnt);

  return InsertResult{u32(old_len), true};
}

}  // namespace sfc::collections
