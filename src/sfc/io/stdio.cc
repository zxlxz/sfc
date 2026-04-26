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
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  void flush() {
    // Stderr is unbuffered, so just do nothing here.
  }

  auto write_str(Str s) -> Result<> {
    auto buf = s.as_bytes();
    while (!buf.is_empty()) {
      const auto nwrite = _TRY(_inn.write(buf));
      if (nwrite == 0) {
        return Error{ErrorKind::WriteZero};
      }
      buf = buf[{nwrite, $}];
    }
    return {};
  }

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

auto Stdout::is_terminal() -> bool {
  return sys::Stdout::is_console();
}

auto Stdout::lock() -> Lock {
  return Lock{Inn::instance()};
}

Stdout::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn.lock()} {}

Stdout::Lock::~Lock() noexcept {
  _inn.flush();
}

void Stdout::Lock::flush() {
  _inn.flush();
}

void Stdout::Lock::write_str(Str s) {
  (void)_inn.write_str(s);
}

auto Stderr::is_terminal() -> bool {
  return sys::Stdout::is_console();
}

auto Stderr::lock() -> Lock {
  return Lock{Inn::instance()};
}

Stderr::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn.lock()} {}

Stderr::Lock::~Lock() {}

void Stderr::Lock::flush() {
  _inn.flush();
}

void Stderr::Lock::write_str(Str s) {
  (void)_inn.write_str(s);
}

}  // namespace sfc::io
