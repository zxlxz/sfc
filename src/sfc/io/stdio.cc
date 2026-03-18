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
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_terminal() const noexcept -> bool {
    return sys::Stdout::is_console();
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

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

class Stderr::Inn {
  sys::Stderr _inn{};
  sync::ReentrantLock _mtx{};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  auto is_terminal() const -> bool {
    return _inn.is_console();
  }

  auto write(Slice<const u8> s) -> Result<usize> {
    return _inn.write(s);
  }

  void flush() noexcept {}

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

auto Stdout::is_terminal() -> bool {
  auto& inn = Inn::instance();
  auto lock = inn.lock();
  return inn.is_terminal();
}

void Stdout::flush() {
  auto& inn = Inn::instance();
  auto lock = inn.lock();
  return inn.flush();
}

void Stdout::write_str(Str s) {
  auto& inn = Inn::instance();
  auto lock = inn.lock();
  const auto bytes = s.as_bytes();
  (void)inn.write(bytes);
}

auto Stdout::lock() -> Lock {
  return Lock{Inn::instance()};
}

Stdout::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn.lock()} {}

Stdout::Lock::~Lock() noexcept {
  _inn.flush();
}

auto Stdout::Lock::is_terminal() -> bool {
  static auto& inn = Inn::instance();
  return inn.is_terminal();
}

void Stdout::Lock::flush() {
  return _inn.flush();
}

void Stdout::Lock::write_str(Str s) {
  const auto bytes = s.as_bytes();
  (void)_inn.write(bytes);
}

auto Stderr::is_terminal() -> bool {
  auto& inn = Inn::instance();
  return inn.is_terminal();
}

void Stderr::flush() {
  auto& inn = Inn::instance();
  return inn.flush();
}

void Stderr::write_str(Str s) {
  auto& inn = Inn::instance();
  (void)inn.write(s.as_bytes());
}

auto Stderr::lock() -> Lock {
  return Lock{Inn::instance()};
}

Stderr::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn.lock()} {}

Stderr::Lock::~Lock() {
  _inn.flush();
}

auto Stderr::Lock::is_terminal() -> bool {
  return _inn.is_terminal();
}

void Stderr::Lock::flush() {
  return _inn.flush();
}

void Stderr::Lock::write_str(Str s) {
  (void)_inn.write(s.as_bytes());
}

}  // namespace sfc::io
