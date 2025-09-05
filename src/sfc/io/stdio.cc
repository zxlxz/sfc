#include "sfc/io/stdio.h"

#include "sfc/io/file.h"
#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

class Stdout::Inn {
  friend class Stdout;
  friend class Stdout::Lock;
  static constexpr usize BUF_SIZE = 4096U;

  sync::ReentrantLock _mtx{};
  File _imp{File::from_fd(sys::io::stdout())};
  Vec<u8> _buf{Vec<u8>::with_capacity(2 * BUF_SIZE)};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_tty() const noexcept -> bool {
    const auto fd = _imp.as_fd();
    const auto res = sys_imp::is_tty(fd);
    return res;
  }

  void flush() noexcept {
    if (_buf.is_empty()) {
      return;
    }

    _imp.write_all(_buf.as_slice());
    _buf.clear();
  }

  void write(Slice<const u8> s) {
    if (s.len() > BUF_SIZE) {
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

    if (_buf.len() > BUF_SIZE) {
      this->flush();
    }
  }
};

class Stderr::Inn {
  friend class Stderr;
  friend class Stderr::Lock;

  sync::ReentrantLock _mtx{};
  File _imp{File::from_fd(sys::io::stderr())};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_tty() const -> bool {
    const auto fd = _imp.as_fd();
    const auto res = sys_imp::is_tty(fd);
    return res;
  }

  void flush() noexcept {}

  void write(Slice<const u8> s) noexcept {
    _imp.write_all(s);
  }
};

Stdout::Lock::Lock() : _inn{Inn::instance()}, _lock{_inn._mtx.lock()} {}

Stdout::Lock::~Lock() noexcept {
  _inn.flush();
}

auto Stdout::Lock::is_tty() -> bool {
  static auto& inn = Inn::instance();
  return inn.is_tty();
}

void Stdout::Lock::flush() {
  return _inn.flush();
}

void Stdout::Lock::write_str(Str s) {
  return _inn.write(s.as_bytes());
}

Stderr::Lock::Lock() : _inn{Inn::instance()}, _lock{_inn._mtx.lock()} {}

Stderr::Lock::~Lock() noexcept {}

auto Stderr::Lock::is_tty() -> bool {
  static auto& inn = Inn::instance();
  return inn.is_tty();
}

void Stderr::Lock::flush() {
  return _inn.flush();
}

void Stderr::Lock::write_str(Str s) {
  return _inn.write(s.as_bytes());
}

}  // namespace sfc::io
