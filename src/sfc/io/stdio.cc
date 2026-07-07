#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

#include "sfc/io/buf.h"
#include "sfc/io/stdio.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class Stdout::Inn {
  BufWriter<sys::Stdout> _inn{{}};
  sync::ReentrantLock _mtx{};

 public:
  static auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_terminal() -> bool {
    return _inn.inner().is_console();
  }

  void flush() noexcept {
    (void)_inn.flush();
  }

  auto write_str(Str s) -> Result<> {
    const auto p = s.rfind('\n');
    if (!p) {
      return _inn.write_str(s);
    }

    const auto [a, b] = s.split_at(*p);
    _TRY(_inn.write_str(a));
    _TRY(_inn.flush());
    return _inn.write_str(b);
  }

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

class Stderr::Inn {
  sys::Stderr _inn{};
  sync::ReentrantLock _mtx{};

 public:
  static auto instance() -> Inn& {
    static auto res = Stderr::Inn{};
    return res;
  }

  auto is_terminal() -> bool {
    return _inn.is_console();
  }

  void flush() {
    // Stderr is unbuffered, so just do nothing here.
  }

  auto write_str(Str s) -> Result<> {
    auto buf = s.as_bytes();
    while (!buf.is_empty()) {
      const auto nwrite = _TRY(_inn.write(buf));
      if (nwrite == 0) {
        return Error::WriteZero;
      }
      buf = buf[{nwrite, buf._len}];
    }
    return Ok{};
  }

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

auto Stdout::is_terminal() -> bool {
  auto& inn = Inn::instance();
  return inn.is_terminal();
}

auto Stdout::lock() -> LockGuard {
  return LockGuard{Inn::instance()};
}

void Stdout::flush() {
  return this->lock().flush();
}

void Stdout::write_str(Str s) {
  return this->lock().write_str(s);
}

Stdout::LockGuard::LockGuard(Inn& inn) : _lock{inn.lock()} {}

Stdout::LockGuard::~LockGuard() noexcept {}

void Stdout::LockGuard::flush() {
  auto& inn = Inn::instance();
  return inn.flush();
}

void Stdout::LockGuard::write_str(Str s) {
  auto& inn = Inn::instance();
  (void)inn.write_str(s);
}

auto Stderr::is_terminal() -> bool {
  auto& inn = Inn::instance();
  return inn.is_terminal();
}

void Stderr::flush() {
  this->lock().flush();
}

void Stderr::write_str(Str s) {
  this->lock().write_str(s);
}

auto Stderr::lock() -> LockGuard {
  return LockGuard{Inn::instance()};
}

Stderr::LockGuard::LockGuard(Inn& inn) : _lock{inn.lock()} {}

Stderr::LockGuard::~LockGuard() noexcept {}

void Stderr::LockGuard::flush() {
  auto& inn = Inn::instance();
  return inn.flush();
}

void Stderr::LockGuard::write_str(Str s) {
  auto& inn = Inn::instance();
  (void)inn.write_str(s);
}

}  // namespace sfc::io
