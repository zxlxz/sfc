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

auto String::as_mut_buf() noexcept -> Buf& {
  return _buf;
}

auto String::as_slice() const noexcept -> Slice<const u8> {
  return _buf.as_slice();
}

auto String::as_mut_slice() noexcept -> Slice<u8> {
  return _buf.as_mut_slice();
}

auto String::as_str() const noexcept -> Str {
  return Str::from_utf8(_buf.as_slice());
}

auto String::operator[](ops::Range ids) const noexcept -> Str {
  const auto v = _buf[ids];
  return Str::from_utf8(v);
}

auto String::operator*() const noexcept -> Str {
  return Str::from_utf8(_buf.as_slice());
}

auto String::operator==(Str other) const noexcept -> bool {
  return this->as_str() == other;
}

auto String::clone() const noexcept -> String {
  return String::from(this->as_str());
}

void String::write_str(Str s) noexcept {
  this->push_str(s);
}

void String::fmt(fmt::Formatter& f) const {
  this->as_str().fmt(f);
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
