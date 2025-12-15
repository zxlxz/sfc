#include "sfc/io/stdio.h"

#include "sfc/io/file.h"
#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

class Stdout::Inn {
  friend class Stdout;
  friend class Stdout::Lock;

  sync::ReentrantLock _mtx{};
  BufWriter<File> _inn = BufWriter{File{sys::io::stdout()}};

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
    (void)_inn.flush();
  }

  auto write(Slice<const u8> s) -> Result<usize> {
    const auto p = s.iter().rposition([](char c) { return c == '\n'; });
    if (!p) {
      return _inn.write(s);
    }

    const auto [a, b] = s.split_at(*p + 1);
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
  File _imp{sys::io::stderr()};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_tty() const -> bool {
    const auto res = sys_imp::is_tty(_imp._fd);
    return res;
  }

  void flush() noexcept {}

  void write(Slice<const u8> s) noexcept {
    (void)_imp.write(s);
  }
};

Stdout::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn._mtx.lock()} {}

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
  (void)_inn.write(s.as_bytes());
}

auto Stdout::lock() -> Lock {
  static auto& inn = Inn::instance();
  return Lock{inn};
}

Stderr::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn._mtx.lock()} {}

Stderr::Lock::~Lock() {
  _inn.flush();
}

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

auto Stderr::lock() -> Lock {
  static auto& inn = Inn::instance();
  return Lock{inn};
}

}  // namespace sfc::io
