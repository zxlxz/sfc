#pragma once

#include "mod.h"

namespace sfc::chr {

[[sfc_inline]] inline auto is_digit(char c) -> bool {
  return '0' <= c && c <= '9';
}

[[sfc_inline]] inline auto is_whitespace(char c) -> bool {
  return c == ' ' || ('\x09' <= c && c <= '\x0d');
};

}  // namespace sfc::chr
