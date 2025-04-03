#include "string.h"

namespace sfc::string {

auto String::pop() -> Option<char> {
  return _vec.pop();
}

void String::push(char c) {
  this->reserve(1);
  _vec.push(c);
}

void String::push_str(Str s) {
  this->reserve(s.len() + 1);
  _vec.extend_from_slice(s.as_chars());
}

void String::write_str(Str s) {
  this->reserve(s.len() + 1);
  _vec.extend_from_slice(s.as_chars());
}

void String::clear() {
  _vec.clear();
}

void String::reserve(usize additional) {
  if (additional == 0) {
    return;
  }
  _vec.reserve(additional + 1);
  _vec.as_mut_ptr()[_vec.len()] = 0;
}

void String::truncate(usize len) {
  if (len >= _vec.len()) {
    return;
  }
  _vec.truncate(len);
  _vec.as_mut_ptr()[_vec.len()] = 0;
}

auto String::remove(usize idx) -> char {
  auto res = _vec.remove(idx);

  _vec.as_mut_slice()[_vec.len()] = 0;
  return res;
}

void String::drain(Range range) {
  _vec.drain(range);
  if (!_vec.is_empty()) {
    _vec.as_mut_slice()[_vec.len()] = 0;
  }
}

void String::insert(usize idx, char ch) {
  this->reserve(1);
  _vec.insert(idx, ch);
}

void String::insert_str(usize idx, Str str) {
  this->reserve(str.len());

  const auto len = this->len();
  const auto pos = cmp::min(idx, len);
  ptr::copy(_vec.as_ptr() + pos, _vec.as_mut_ptr() + pos + str.len(), len - pos);
  ptr::copy_nonoverlapping(str.as_ptr(), _vec.as_mut_ptr() + pos, str.len());
  _vec.set_len(_vec.len() + str.len());
}

}  // namespace sfc::string
