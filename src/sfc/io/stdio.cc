#include "sfc/io/stdio.h"

#include "sfc/sync/mutex.h"
#include "sfc/sys/io.h"

namespace sfc::io {

class Stdout::Inn {
  friend class Stdout;
  friend class Stdout::Lock;
  static constexpr auto BUF_CAP = 4096U;

  sync::ReentrantLock _mtx{};
  sys::io::File _imp{sys::io::stdout()};
  String _buf{String::with_capacity(BUF_CAP * 2)};

 public:
  static inline auto instance() -> Inn& {
    static auto res = Inn{};
    return res;
  }

  void flush() {
    if (_buf.is_empty()) {
      return;
    }

    _imp.write(_buf.as_ptr(), _buf.len());
    _buf.clear();
  }

  void write_str(Str s) {
    this->write_buf(s);

    const auto pos = _buf.rfind('\n');
    if (!pos) {
      return;
    }

    const auto n = pos.unwrap();
    _imp.write(_buf.as_ptr(), n + 1);
    _buf.drain({0, n + 1});
  }

 private:
  void write_buf(Str s) {
    if (s.len() > BUF_CAP) {
      this->flush();
      _imp.write(s.as_ptr(), s.len());
      return;
    }

    _buf.write_str(s);
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
  return _inn._imp.is_tty();
}

void Stdout::flush() {
  auto lock = _inn._mtx.lock();
  return _inn.flush();
}

void Stdout::write_str(Str s) {
  auto lock = _inn._mtx.lock();
  _inn.write_str(s);
}

Stdout::Lock::Lock(Inn& inn) : _inn{inn}, _lock{_inn._mtx.lock()} {}

Stdout::Lock::~Lock() {
  _inn.flush();
}

void Stdout::Lock::flush() {
  return _inn.flush();
}

void Stdout::Lock::write_str(Str s) {
  return _inn.write_str(s);
}

}  // namespace sfc::io
