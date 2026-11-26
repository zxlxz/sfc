#include "sfc/io/buf.h"
#include "sfc/io/stdio.h"
#include "sfc/sys/io.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

struct RawStdout {
  static auto write(Slice<const u8> buf) -> io::Result<usize> {
    return sys::Stdout().write(buf);
  }

  static auto flush() -> io::Result<> {
    return Ok{};
  }

  static auto is_terminal() -> bool {
    return sys::Stdout().is_terminal();
  }
};

struct RawStderr {
  static auto write(Slice<const u8> buf) -> io::Result<usize> {
    return sys::Stderr().write(buf);
  }

  static auto flush() -> io::Result<> {
    return Ok{};
  }

  static auto is_terminal() -> bool {
    return sys::Stderr().is_terminal();
  }
};

class Stdout::Inn {
  RawStdout _out{};
  u8 _buf[256] = {};
  BufWriter _write{BufWriter::new_(_out, _buf)};
  sync::ReentrantLock _mtx{};

 public:
  Inn() {}

  ~Inn() {}

  static auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

 public:
  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }

  auto is_terminal() -> bool {
    return _out.is_terminal();
  }

  void flush() noexcept {
    (void)_write.flush();
  }

  auto write_str(Str s) -> Result<> {
    if (s.is_empty()) {
      return Ok{};
    }

    const auto p = s.rfind('\n');
    if (!p) {
      return _write.write_str(s);
    }

    const auto [a, b] = s.split_at(*p + 1);
    _TRY(_write.write_str(a));
    _TRY(_write.flush());
    _TRY(_write.write_str(b));
    return Ok{};
  }
};

class Stderr::Inn {
  RawStderr _out{};
  sync::ReentrantLock _mtx{};

 public:
  static auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

 public:
  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }

  auto is_terminal() -> bool {
    return _out.is_terminal();
  }

  auto write_str(Str s) -> Result<> {
    auto buf = s.as_bytes();
    while (!buf.is_empty()) {
      const auto nwrite = _TRY(_out.write(buf));
      if (nwrite == 0) {
        return Error::WriteZero;
      }
      buf = buf[{nwrite, buf._len}];
    }
    return Ok{};
  }

  auto flush() -> Result<> {
    return Ok{};
  }
};

auto Stdout::is_terminal() -> bool {
  return RawStdout::is_terminal();
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
  return RawStderr::is_terminal();
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

void Stderr::LockGuard::flush() {
  (void)_inn.flush();
}

void Stderr::LockGuard::write_str(Str s) {
  (void)_inn.write_str(s);
}

}  // namespace sfc::io
