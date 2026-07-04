#include "sfc/alloc/string.h"
#include "sfc/core/chr.h"

namespace sfc::string {

auto String::with_capacity(usize capacity) noexcept -> String {
  auto res = String{};
  res.reserve(capacity);
  return res;
}

auto String::from(Str s) noexcept -> String {
  auto res = String{};
  res.push_str(s);
  return res;
}

auto String::from_buf(Buf buf) -> String {
  auto res = String{};
  res._buf = mem::move(buf);
  return res;
}

void String::push_str(Str s) noexcept {
  if (s._len == 0) return;
  _buf.extend_from_slice(s.as_bytes());
}

void String::reserve(usize amt) noexcept {
  _buf.reserve(amt);
}

void String::truncate(usize len) noexcept {
  _buf.truncate(len);
}

void String::clear() noexcept {
  _buf.clear();
}

auto String::pop() noexcept -> Option<char32_t> {
  if (_buf.is_empty()) {
    return {};
  }

  auto chars = this->as_str().chars();
  const auto ret = chars.next_back();
  const auto len = num::cast_unsigned(chars._end - chars._ptr);
  _buf.truncate(len);
  return ret;
}

void String::push(char32_t c) noexcept {
  // ansii code point, just push as u8
  if (c < 0x80) {
    const auto byte = static_cast<u8>(c);
    _buf.push(byte);
    return;
  }

  u8 buf[4] = {};
  const auto code_len = chr::utf8_encode(buf, c);
  _buf.extend_from_slice({buf, code_len});
}

}  // namespace sfc::string
