#include "string.h"

namespace sfc::string {

auto String::pop() -> Option<char> {
  return _vec.pop();
}

void String::push(char c) {
  _vec.push(c);
}

void String::push_str(Str s) {
  _vec.extend_from_slice(s.as_chars());
}

void String::write_str(Str s) {
  _vec.extend_from_slice(s.as_chars());
}

void String::clear() {
  _vec.clear();
}

void String::reserve(usize additional) {
  _vec.reserve(additional);
}

void String::truncate(usize len) {
  _vec.truncate(len);
}

auto String::remove(usize idx) -> char {
  auto res = _vec.remove(idx);
  return res;
}

void String::drain(Range<> range) {
  _vec.drain(range);
}

void String::insert(usize idx, char ch) {
  _vec.insert(idx, ch);
}

void String::insert_str(usize idx, Str str) {
  const auto len = this->len();
  const auto amt = str.len();
  idx = cmp::min(idx, len);

  _vec.reserve(amt);
  ptr::copy(_vec.as_ptr() + idx, _vec.as_mut_ptr() + idx + amt, len - idx);
  ptr::copy_nonoverlapping(str.as_ptr(), _vec.as_mut_ptr() + idx, amt);
  _vec.set_len(amt);
}

}  // namespace sfc::string
