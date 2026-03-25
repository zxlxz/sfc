#pragma once

#include "sfc/core/str/mod.h"

namespace sfc::str {

struct Chars : iter::Iterator<char32_t> {
  const char* _ptr;
  const char* _end;

 public:
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

inline auto Str::chars() const noexcept -> Chars {
  return {{}, _ptr, _ptr + _len};
}

}  // namespace sfc::str
