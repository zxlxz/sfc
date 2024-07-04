#include <stdlib.h>

#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

template <trait::UInt T>
static auto parse_int(Slice<const char> s) -> Option<T> {
  auto end = const_cast<char*>(s._ptr + s._len);
  auto val = ::strtoul(s._ptr, &end, 10);

  if (val > num::max_value<T>()) {
    return {};
  }
  if (end != s._ptr + s._len) {
    return {};
  }
  return static_cast<T>(val);
}

template <trait::SInt T>
static auto parse_int(Slice<const char> s) -> Option<T> {
  auto end = const_cast<char*>(s._ptr + s._len);
  auto val = ::strtol(s._ptr, &end, 10);

  if (val > num::max_value<T>() || val < num::min_value<T>()) {
    return {};
  }
  if (end != s._ptr + s._len) {
    return {};
  }
  return static_cast<T>(val);
}

template <class T>
auto Str::parse() const -> Option<T> {
  char buf[64];
  if (_len == 0 || _len >= sizeof(buf)) {
    return {};
  }
  ptr::copy(_ptr, buf, _len);
  buf[_len] = 0;

  const auto ret = parse_int<T>({buf, _len});
  return ret;
}

template auto Str::parse<signed char>() const -> Option<signed char>;
template auto Str::parse<signed short>() const -> Option<signed short>;
template auto Str::parse<signed int>() const -> Option<signed int>;
template auto Str::parse<signed long>() const -> Option<signed long>;
template auto Str::parse<signed long long>() const -> Option<signed long long>;

template auto Str::parse<unsigned char>() const -> Option<unsigned char>;
template auto Str::parse<unsigned short>() const -> Option<unsigned short>;
template auto Str::parse<unsigned int>() const -> Option<unsigned int>;
template auto Str::parse<unsigned long>() const -> Option<unsigned long>;
template auto Str::parse<unsigned long long>() const -> Option<unsigned long long>;

}  // namespace sfc::str
