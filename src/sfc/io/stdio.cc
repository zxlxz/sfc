#include "sfc/io/stdio.h"

#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

auto Stdout::lock() -> sync::LockGuard {
  static auto mtx = sync::Mutex{};
  return mtx.lock();
}

auto Stdout::is_tty() -> bool {
  static auto& imp = sys_imp::stdout();
  return imp.is_tty();
}

void Stdout::flush() {
  static auto& imp = sys_imp::stdout();
  imp.flush();
}

void Stdout::write_str(Str s) {
  static auto& imp = sys_imp::stdout();
  imp.write(s.as_ptr(), s.len());
}

}  // namespace sfc::io
