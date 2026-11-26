#include "sfc/alloc/string.h"

namespace sfc::string {

String::String(List<u8> buf) noexcept : _buf{mem::move(buf)} {}

String::~String() = default;

auto String::new_() -> String {
  return String{List<u8>::new_()};
}

auto String::with_capacity(usize capacity) -> String {
  return String{List<u8>::with_capacity(capacity)};
}

auto String::from(Str s) -> String {
  auto res = String::with_capacity(s._len);
  res.push_str(s);
  return res;
}

auto String::from_utf8(List<u8> buf) -> String {
  return String{mem::move(buf)};
}

auto String::from_utf16(Slice<const u16> buf) -> String {
  auto res = String::with_capacity(buf.len());
  chr::decode_utf16(buf).for_each([&](char32_t c) { res.push(c); });
  return res;
}

auto String::from_wstr(Slice<const wchar_t> buf) -> String {
  auto res = String::with_capacity(buf.len());
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    buf.iter().for_each([&](auto ch) { res.push(ch); });
  } else if constexpr (sizeof(wchar_t) == sizeof(u16)) {
    auto utf16_buf = Slice{(const u16*)buf._ptr, buf._len};
    chr::decode_utf16(utf16_buf).for_each([&](char32_t c) { res.push(c); });
  }
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
  return Str::from_utf8(_buf[ids]);
}

auto String::pop() noexcept -> Option<char32_t> {
  if (_buf.is_empty()) {
    return None{};
  }

  auto chars = this->as_str().chars();
  const auto ret = chars.next_back();
  _buf.set_len(chars._buf._len);
  return ret;
}

void String::push(char32_t c) noexcept {
  const auto s = chr::encode_utf8(c);
  _buf.extend_from_slice({s.data, s.len});
}

void String::push_str(Str s) noexcept {
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

auto String::operator==(const String& other) const noexcept -> bool {
  return this->as_str() == other.as_str();
}

auto String::operator==(Str other) const noexcept -> bool {
  return this->as_str() == other;
}

String::operator Str() const noexcept {
  return this->as_str();
}

auto String::operator*() const noexcept -> Str {
  return this->as_str();
}

auto String::clone() const noexcept -> String {
  return String{_buf.clone()};
}

void String::write_str(Str s) noexcept {
  this->push_str(s);
}

}  // namespace sfc::string
