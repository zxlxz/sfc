#include "sfc/io/stdio.h"

#include "sfc/io/file.h"
#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

class Stdout::Inn {
  friend class Stdout;
  friend class Stdout::Lock;

  static constexpr auto BUF_CAP = 4096U;

  sync::ReentrantLock _mtx{};
  File _imp{sys::io::stdout()};
  Vec<u8> _buf{Vec<u8>::with_capacity(BUF_CAP * 2)};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_tty() const -> bool {
    const auto res = sys_imp::is_tty(_imp.as_fd());
    return res;
  }

  void flush() {
    if (_buf.is_empty()) {
      return;
    }

    _imp.write_all(_buf.as_slice());
    _buf.clear();
  }

  void write(Slice<const u8> s) {
    if (s.len() > BUF_CAP) {
      this->flush();
      _imp.write_all(s);
      return;
    }

    if (const auto pos = s.rfind('\n')) {
      const auto [a, b] = s.split_at(*pos);
      _buf.extend_from_slice(a);
      this->flush();
      _buf.extend_from_slice(b);
    } else {
      _buf.extend_from_slice(s);
    }

    if (_buf.len() > BUF_CAP) {
      this->flush();
    }
  }
};

Stdout::Stdout() : _inn{Inn::instance()} {}

Stdout::~Stdout() {}

auto Stdout::lock() -> Lock {
  return Lock{_inn};
}

auto Stdout::is_tty() -> bool {
  return _inn.is_tty();
}

void Stdout::flush() {
  auto lock = _inn._mtx.lock();
  return _inn.flush();
}

void Stdout::write_str(Str s) {
  auto lock = _inn._mtx.lock();
  _inn.write(s.as_bytes());
}

Stdout::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn._mtx.lock()} {}

Stdout::Lock::~Lock() {
  _inn.flush();
}

void Stdout::Lock::flush() {
  return _inn.flush();
}

void Stdout::Lock::write_str(Str s) {
  return _inn.write(s.as_bytes());
}

}  // namespace sfc::io
