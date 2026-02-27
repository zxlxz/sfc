#include "sfc/alloc/string.h"

namespace sfc::string {

static auto utf8_encode(u8 (&buf)[4], char32_t c) -> Slice<const u8> {
  if (c <= 0x7F) {
    buf[0] = static_cast<u8>(c);
    return {buf, 1};
  }

  if (c <= 0x7FF) {
    buf[0] = static_cast<u8>(0xC0 | (c >> 6));
    buf[1] = static_cast<u8>(0x80 | (c & 0x3F));
    return {buf, 2};
  }

  if (c <= 0xFFFF) {
    buf[0] = static_cast<u8>(0xE0 | (c >> 12));
    buf[1] = static_cast<u8>(0x80 | ((c >> 6) & 0x3F));
    buf[2] = static_cast<u8>(0x80 | (c & 0x3F));
    return {buf, 3};
  }

  if (c <= 0x10FFFF) {
    buf[0] = static_cast<u8>(0xF0 | (c >> 18));
    buf[1] = static_cast<u8>(0x80 | ((c >> 12) & 0x3F));
    buf[2] = static_cast<u8>(0x80 | ((c >> 6) & 0x3F));
    buf[3] = static_cast<u8>(0x80 | (c & 0x3F));
    return {buf, 4};
  }

  return {buf, 0};
}

void String::push_str(Str s) noexcept {
  if (s._len == 0) {
    return;
  }
  _vec.extend_from_slice(s.as_bytes());
}

void String::reserve(usize amt) noexcept {
  _vec.reserve(amt);
}

void String::truncate(usize len) noexcept {
  _vec.truncate(len);
}

void String::clear() noexcept {
  _vec.clear();
}

auto String::pop() noexcept -> Option<char32_t> {
  if (_vec.is_empty()) {
    return {};
  }

  auto chars = this->as_str().chars();

  const auto ret = chars.next_back();
  _vec.truncate(static_cast<u32>(chars._end - chars._ptr));
  return ret;
}

void String::push(char32_t c) noexcept {
  if (c < 0x80) {
    _vec.push(static_cast<u8>(c));
  } else {
    u8 buf[4] = {};
    const auto bytes = utf8_encode(buf, c);
    _vec.extend_from_slice(bytes);
  }
}

}  // namespace sfc::string
