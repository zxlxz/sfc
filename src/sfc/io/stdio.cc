#include "sfc/io/stdio.h"

#include "sfc/io/file.h"
#include "sfc/io/buf.h"
#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

class Stdout::Inn {
  friend class Stdout;
  friend class Stdout::Lock;

  sync::ReentrantLock _mtx{};
  BufWriter<File> _inn = BufWriter{File::from_fd(sys::io::stdout())};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_tty() const noexcept -> bool {
    const auto fd = sys::io::stdout();
    const auto res = sys_imp::is_tty(fd);
    return res;
  }

  void flush() noexcept {
    _inn.flush();
  }

  auto write(Slice<const u8> s) -> Result<usize> {
    const auto p = s.iter().rposition([](char c) { return c == '\n'; });
    if (!p) {
      return _inn.write(s);
    }

    const auto [a, b] = s.split_at(p.unwrap() + 1);
    _TRY(_inn.write(a));
    _TRY(_inn.flush());
    _TRY(_inn.write(b));
    return s.len();
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
  _inn.write(s.as_bytes());
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
