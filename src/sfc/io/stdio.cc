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
  static thread_local String buf{};
  buf.push_str(s);

  auto& imp = sys_imp::Stdout::instance();
  if (auto pos = buf.rfind('\n')) {
    const auto n = *pos + 1;
    imp.write_str(buf[{0UL, n}]);
    buf.drain({0, n});
  }
}

}  // namespace sfc::io
