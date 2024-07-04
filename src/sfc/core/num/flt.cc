#include <stdlib.h>

#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

template <class T>
static auto parse_flt(Slice<const char> s) -> Option<T> {
  auto end = const_cast<char*>(s._ptr + s._len);
  auto val = ::strtod(s._ptr, &end);

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

  const auto ret = parse_flt<T>({buf, _len});
  return ret;
}

template auto Str::parse<f32>() const -> Option<f32>;
template auto Str::parse<f64>() const -> Option<f64>;

}  // namespace sfc::str
