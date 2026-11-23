#include "str_set.h"

namespace sfc::collections {

namespace detail {

static auto bytes_equal(const void* a, const void* b, usize n) {
  return n == 0 || __builtin_memcmp(a, b, n) == 0;
}

}  // namespace detail

auto StrSet::new_() -> StrSet {
  return StrSet{List<u8>{}};
}

auto StrSet::with_capacity(usize capacity) -> StrSet {
  const usize str_capacity = capacity * kAlignSize;
  return StrSet{List<u8>::with_capacity(str_capacity)};
}

// - str is stored as `u8[]` with a NUL terminator and padding to align to kAlignSize
auto StrSet::search(Str s) const -> Option<u32> {
  if (s.is_empty()) {
    return {};
  }

  const auto [haystack_ptr, haystack_len] = _buf.as_slice();
  const auto [needle_ptr, needle_len] = s;

  // _buf contains str with NUL terminator and padding to align to kAlignSize
  // if `s._len < kAlignSize`, we need to copy it into a full-slot buffer for comparison
  char head_buf[kAlignSize] = {0};
  if (needle_len < kAlignSize) {
    __builtin_memcpy(head_buf, needle_ptr, needle_len);
  }

  const auto tail_len = needle_len > kAlignSize ? needle_len - kAlignSize : 0;

  const auto head = needle_len < kAlignSize ? head_buf : needle_ptr;
  const auto tail = needle_ptr + needle_len - tail_len;

  // haystack always contains a NUL-terminated and aligned string
  // so we should check (needle_len + i < haystack_len) to avoid reading past the end
  for (auto i = 0U; i + needle_len < haystack_len; i += kAlignSize) {
    const auto p = haystack_ptr + i;
    // check NUL terminator and alignment
    if (p[needle_len] != 0) {
      continue;
    }
    // check the head of the needle against the current slot
    if (!detail::bytes_equal(p, head, kAlignSize)) {
      continue;
    }
    // check the tail of the needle against the current slot
    if (detail::bytes_equal(p + kAlignSize, tail, tail_len)) {
      return u32(i / kAlignSize);
    }
  }

  return {};
}

void StrSet::reserve(usize add_cnt) {
  _buf.reserve(add_cnt * kAlignSize);
}

auto StrSet::insert(Str s) -> InsertResult {
  sfc::assert_(!s.is_empty(), "StrSet::insert: cannot insert empty str");

  const auto search_idx = this->search(s);
  if (search_idx.is_some()) {
    return InsertResult{*search_idx, false};
  }

  const auto old_len = _buf.len();

  // store as key + '\0' + padding, occupying whole slots;
  // the '\0' must fit inside the needle's aligned slot for search's NUL check
  u8 padding[kAlignSize] = {0};
  const auto aligned_size = num::align_up<kAlignSize>(s.len() + 1);
  const auto padding_len = aligned_size - s.len();

  _buf.reserve(aligned_size);
  _buf.extend_from_slice(s.as_bytes());
  _buf.extend_from_slice(Slice{padding, padding_len});

  const auto ret_idx = u32(old_len / kAlignSize);
  return InsertResult{ret_idx, true};
}

}  // namespace sfc::collections
