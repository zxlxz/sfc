#include "sfc/io/stdio.h"

#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

class Stdout::Inn {
  friend struct Stdout;

  sys_imp::File&      _imp = sys_imp::stdout();
  sync::ReentrantLock _mtx{};
  String              _buf{};

  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto lock() -> sync::ReentrantLockGuard {
    return _mtx.lock();
  }

  auto is_tty() const -> bool {
    return _imp.is_tty();
  }

  void flush() {
    _imp.flush();
  }

  void write_str(Str s) {
    _buf.write_str(s);

    if (auto pos = _buf.rfind('\n')) {
      const auto n = pos.unwrap();
      const auto s = _buf[{0, n}];
      _imp.write(s.as_ptr(), s.len());
      _buf.drain({0, n});
    }
  }
};

auto Stdout::lock() -> StdoutLock {
  static auto  out = Stdout{};
  static auto& inn = Inn::instance();

  return StdoutLock{._impl{out}, ._lock{inn.lock()}};
}

auto Stdout::is_tty() -> bool {
  static auto& inn = Inn::instance();
  return inn.is_tty();
}

void Stdout::flush() {
  static auto& inn = Inn::instance();
  return inn.flush();
}

void Stdout::write_str(Str s) {
  static auto& inn = Inn::instance();
  inn.write_str(s);
}

}  // namespace sfc::io
