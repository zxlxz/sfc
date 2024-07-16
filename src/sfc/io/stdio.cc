#include "stdio.h"

#include "sfc/sys/io.inl"

namespace sfc::io {

namespace sys_imp = sys::io;

auto Stdout::instance() -> Stdout& {
  static Stdout res{};
  return res;
}

auto Stdout::is_tty() const -> bool {
  return sys_imp::Stdout::is_tty();
}

void Stdout::flush() {
  return sys_imp::Stdout::flush();
}

void Stdout::write_str(Str s) {
  sys_imp::Stdout::write_str(s);
}

}  // namespace sfc::io
