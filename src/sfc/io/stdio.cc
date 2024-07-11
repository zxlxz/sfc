#include "stdio.h"

#include "sfc/sys/io.inl"

namespace sfc::io {

namespace sys_imp = sys::io;

auto Stdout::instance() -> Stdout& {
  static Stdout ret{};
  return ret;
}

auto Stdout::is_tty() const -> bool {
  const auto& imp = sys_imp::Stdout::instance();
  return imp.is_tty();
}

void Stdout::write_str(Str s) {
  auto& imp = sys_imp::Stdout::instance();
  imp.write_str(s);
}

}  // namespace sfc::io
