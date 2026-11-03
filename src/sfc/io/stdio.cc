#include "sfc/io/buf.h"
#include "sfc/io/stdio.h"
#include "sfc/sys/io.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

struct Stdout::Inn {
  BufWriter<sys::StdIo> _inn{_inn.new_(sys::StdIo::Output())};
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

    const auto [a, b] = s.split_at(*p + 1);
    _TRY(_inn.write_str(a));
    _TRY(_inn.flush());
    _TRY(_inn.write_str(b));
    return Ok{};
  }

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

struct Stderr::Inn {
  sys::StdIo _inn{sys::StdIo::Error()};
  sync::ReentrantLock _mtx{};

 public:
  static auto instance() -> Inn& {
    static auto res = Stderr::Inn{};
    return res;
  }

  auto is_terminal() -> bool {
    return _inn.is_console();
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
  auto& inn = Inn::instance();
  auto guard = inn.lock();
  return LockGuard{inn, mem::move(guard)};
}

void Stdout::flush() {
  return this->lock().flush();
}

void Stdout::write_str(Str s) {
  return this->lock().write_str(s);
}

auto Stderr::is_terminal() -> bool {
  auto& inn = Inn::instance();
  return inn.is_terminal();
}

auto Stderr::lock() -> LockGuard {
  auto& inn = Inn::instance();
  auto lock = inn.lock();
  return LockGuard{inn, mem::move(lock)};
}

void Stderr::write_str(Str s) {
  return this->lock().write_str(s);
}

void Stderr::flush() {
  return this->lock().flush();
}

Stdout::LockGuard::LockGuard(Inn& inn, sync::ReentrantLock::Guard lock) : _inn{inn}, _lock{mem::move(lock)} {}

Stdout::LockGuard::~LockGuard() noexcept {}

void Stdout::LockGuard::flush() {
  return _inn.flush();
}

void Stdout::LockGuard::write_str(Str s) {
  (void)_inn.write_str(s);
}

Stderr::LockGuard::LockGuard(Inn& inn, sync::ReentrantLock::Guard lock) : _inn{inn}, _lock{mem::move(lock)} {}

Stderr::LockGuard::~LockGuard() noexcept {}

void Stderr::LockGuard::flush() {}

void Stderr::LockGuard::write_str(Str s) {
  (void)_inn.write_str(s);
}

}  // namespace sfc::io
