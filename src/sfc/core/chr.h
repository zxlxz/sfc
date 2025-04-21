#pragma once

#include "sfc/core/mod.h"

namespace sfc::chr {

inline auto is_digit(char c) -> bool {
  return '0' <= c && c <= '9';
}

inline auto is_whitespace(char c) -> bool {
  return c == ' ' || ('\x09' <= c && c <= '\x0d');
};

}  // namespace sfc::chr
