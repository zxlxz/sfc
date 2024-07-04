#include "stdio.h"

#include <unistd.h>

namespace sfc::io {

auto Stdout::instance() -> Stdout& {
  static Stdout ret{};
  return ret;
}

auto Stdout::is_tty() const -> bool {
  const auto res = ::isatty(STDOUT_FILENO);
  return res == 1;
}

void Stdout::write_str(Str s) {
  static thread_local String buf{};

  buf.push_str(s);

  if (auto pos = buf.rfind('\n')) {
    const auto n = *pos + 1;
    __builtin_printf("%.*s", static_cast<int>(n), buf.as_ptr());
    buf.drain({0, n});
  }
}

}  // namespace sfc::io
