#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

#include "sfc/io/buf.h"
#include "sfc/io/stdio.h"
#include "sfc/sync/mutex.h"

namespace sfc::io {

class StdoutImpl {
  BufWriter<sys::Stdout> _inn{{}};
  sync::ReentrantLock _mtx{};

 public:
  static inline auto instance() -> StdoutImpl& {
    static auto res = StdoutImpl{};
    return res;
  }

  static auto is_terminal() -> bool {
    return sys::Stdout::is_console();
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

class StderrImpl {
  sys::Stderr _inn{};
  sync::ReentrantLock _mtx{};

 public:
  static auto instance() -> StderrImpl& {
    static auto res = StderrImpl{};
    return res;
  }

  static auto is_terminal() -> bool {
    return sys::Stderr::is_console();
  }

  void flush() {
    // Stderr is unbuffered, so just do nothing here.
  }

  auto write_str(Str s) -> Result<> {
    auto buf = s.as_bytes();
    while (!buf.is_empty()) {
      const auto nwrite = _TRY(_inn.write(buf));
      if (nwrite == 0) {
        return {Error::WriteZero};
      }
      buf = buf[{nwrite, buf._len}];
    }
    return {};
  }

  auto lock() noexcept -> sync::ReentrantLock::Guard {
    return _mtx.lock();
  }
};

StdoutLock::StdoutLock(StdoutImpl& imp) : _lock{imp.lock()} {}

StdoutLock::~StdoutLock() noexcept {}

void StdoutLock::flush() {
  static auto& imp = StdoutImpl::instance();
  imp.flush();
}

void StdoutLock::write_str(Str s) {
  static auto& imp = StdoutImpl::instance();
  (void)imp.write_str(s);
}

StderrLock::StderrLock(StderrImpl& imp) : _lock{imp.lock()} {}

StderrLock::~StderrLock() noexcept {}

void StderrLock::flush() {
  static auto& imp = StderrImpl::instance();
  imp.flush();
}

void StderrLock::write_str(Str s) {
  static auto& imp = StderrImpl::instance();
  (void)imp.write_str(s);
}

auto Stdout::is_terminal() -> bool {
  return StdoutImpl::is_terminal();
}

auto Stdout::lock() -> StdoutLock {
  return StdoutLock{StdoutImpl::instance()};
}

auto Stderr::is_terminal() -> bool {
  return StderrImpl::is_terminal();
}

auto Stderr::lock() -> StderrLock {
  return StderrLock{StderrImpl::instance()};
}

}  // namespace sfc::io
