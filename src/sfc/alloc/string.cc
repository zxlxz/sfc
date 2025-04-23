#include "sfc/alloc/string.h"

namespace sfc::string {

auto String::pop() -> Option<char> {
  auto ret = _vec.pop();
  return ret;
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
  if (len >= _vec.len()) {
    return;
  }
  _vec.truncate(len);
}

auto String::remove(usize idx) -> char {
  const auto res = _vec[idx];
  this->drain({idx, idx + 1});
  return res;
}

void String::set_len(usize new_len) {
  if (new_len > _vec.capacity()) {
    return;
  }
  _vec.set_len(new_len);
}

void String::drain(Range range) {
  _vec.drain(range);
}

void String::insert(usize idx, char ch) {
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
