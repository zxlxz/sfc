#include "stdio.h"

#include "sfc/sys/io.inl"

namespace sfc::io {

namespace sys_imp = sys::io;

static auto sys_stdout() -> sys_imp::Stdout& {
  static auto res = sys_imp::Stdout{};
  return res;
}

auto Stdout::is_tty() -> bool {
  static auto& imp = sys_stdout();
  return imp.is_tty();
}

void Stdout::flush() {
  static auto& imp = sys_stdout();
  imp.flush();
}

void Stdout::write_str(Str s) {
  static auto& imp = sys_stdout();
  imp.write(s.as_bytes());
}

}  // namespace sfc::io
