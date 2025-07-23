#include "sfc/alloc/string.h"

namespace sfc::string {

auto String::pop() -> Option<char> {
  auto ret = _vec.pop();
  return ret;
}

void String::push(char c) {
  _vec.reserve(2);
  _vec.push(c);
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

void String::push_str(Str s) {
  if (s._len == 0) {
    return;
  }

  _vec.reserve(s._len + 1);
  _vec.extend_from_slice(s.as_chars());
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

void String::write_str(Str s) {
  if (s._len == 0) {
    return;
  }

  _vec.reserve(s._len + 1);
  _vec.extend_from_slice(s.as_chars());
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

void String::clear() {
  _vec.clear();
}

void String::reserve(usize additional) {
  _vec.reserve(additional + 1);
}

void String::truncate(usize len) {
  if (len >= _vec.len()) {
    return;
  }
  _vec.truncate(len);
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

auto String::remove(usize idx) -> char {
  const auto c = _vec.remove(idx);
  _vec.get_unchecked_mut(_vec.len()) = 0;
  return c;
}

void String::drain(Range ids) {
  if (_vec.is_empty() || ids.len() == 0) {
    return;
  }

  _vec.drain(ids);
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

void String::insert(usize idx, char ch) {
  _vec.reserve(2);
  _vec.insert(idx, ch);
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

void String::insert_str(usize idx, Str str) {
  if (str.is_empty()) {
    return;
  }

  const auto len = _vec.len();
  panicking::assert(idx <= len, "String::insert_str: idx({}) out of range([0,{}))", idx, len);

  _vec.reserve(str._len + 1);
  _vec.set_len(len + str._len);

  const auto p = _vec.as_mut_ptr() + idx;
  ptr::copy(p, p + str._len, len - idx);
  ptr::copy_nonoverlapping(str._ptr, p, str._len);
  _vec.get_unchecked_mut(_vec.len()) = 0;
}

}  // namespace sfc::string
