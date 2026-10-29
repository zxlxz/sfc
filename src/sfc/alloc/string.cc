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

auto String::from_utf8(List<u8> v) -> String {
  auto res = String{};
  res._buf = mem::move(v);
  return res;
}

auto String::from_utf16(Slice<const u16> v) -> String {
  if (v.is_empty()) {
    return String{};
  }

  auto res = String{};
  res.reserve(v.len());
  chr::decode_utf16(v).for_each([&](char32_t c) { res.push(c); });
  return res;
}

auto String::capacity() const noexcept -> usize {
  return _buf.capacity();
}

auto String::len() const noexcept -> usize {
  return _buf.len();
}

auto String::is_empty() const noexcept -> bool {
  return _buf.is_empty();
}

auto String::as_ptr() const noexcept -> const u8* {
  return _buf.as_ptr();
}

auto String::as_str() const noexcept -> Str {
  return Str::from_utf8(_buf.as_slice());
}

auto String::as_bytes() const noexcept -> Slice<const u8> {
  return _buf.as_slice();
}

auto String::as_mut_list() noexcept -> List<u8>& {
  return _buf;
}

auto String::operator[](usize idx) const noexcept -> u8 {
  return _buf[idx];
}

auto String::operator[](Range ids) const noexcept -> Str {
  const auto v = _buf[ids];
  return Str::from_utf8(v);
}

auto operator==(const String& a, const String& b) noexcept -> bool {
  return a.as_str() == b.as_str();
}

auto operator==(const String& a, Str b) noexcept -> bool {
  return a.as_str() == b;
}

auto operator==(Str a, const String& b) noexcept -> bool {
  return a == b.as_str();
}

auto String::clone() const noexcept -> String {
  return String::from(this->as_str());
}

void String::write_str(Str s) noexcept {
  this->push_str(s);
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
  const auto ch = chars.next_back();
  _buf.truncate(chars._buf._len);
  return ch;
}

void String::push(char32_t c) noexcept {
  // ansii code point, just push as u8
  if (c < 0x80) {
    _buf.push(u8(c));
    return;
  }

  u8 buf[4] = {};
  const auto v = chr::encode_utf8(buf, c);
  this->push_str(Str::from_utf8(v));
}

}  // namespace sfc::string
