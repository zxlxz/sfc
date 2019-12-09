#pragma once

#include "rc/io/read.h"
#include "rc/io/write.h"
#include "rc/sync/mutex.h"

namespace rc::io {

struct Stdin {
  auto read(Slice<u8> buf) -> usize;
};

struct Stdout {
  struct Static {
    BufWriter<sys::io::Stdout> _inner;
    sync::Mutex _mutex;

   private:
    Static() noexcept : _inner{sys::io::Stdout{}} {}
    Static(const Static&) = delete;

   public:
    static auto instance() -> Static& {
      static auto res = Static{};
      return res;
    }
  };

  static auto write(Slice<const u8> buf) -> usize {
    auto& impl = Static::instance()._inner;
    return impl.write(buf);
  }

  static auto flush() -> void {
    auto& impl = Static::instance()._inner;
    return impl.flush();
  }

  struct Lock {
    sync::MutexGuard _guard;

    ~Lock() {
      auto& impl = Static::instance()._inner;
      impl.flush();
    }

    static auto write(Slice<const u8> buf) -> usize {
      return Stdout::write(buf);
    }
  };

  auto lock() -> Lock {
    auto& impl = Static::instance()._mutex;
    return Lock{impl.lock()};
  }
};

struct Stderr {
  struct Static {
    BufWriter<sys::io::Stderr> _inner;
    sync::Mutex _mutex;

   private:
    Static() noexcept : _inner{sys::io::Stderr{}} {}
    Static(const Static&) = delete;

   public:
    static auto instance() -> Static& {
      static auto res = Static{};
      return res;
    }
  };

  static auto write(Slice<const u8> buf) -> usize {
    auto& impl = Static::instance()._inner;
    return impl.write(buf);
  }

  static auto flush() -> void {
    auto& impl = Static::instance()._inner;
    return impl.flush();
  }

  struct Lock {
    sync::MutexGuard _guard;

    ~Lock() {
      auto& impl = Static::instance()._inner;
      impl.flush();
    }

    static auto write(Slice<const u8> buf) -> usize {
      return Stderr::write(buf);
    }
  };

  auto lock() -> Lock {
    auto& impl = Static::instance()._mutex;
    return Lock{impl.lock()};
  }
};

template <class... T>
void print(const T&... args) {
  auto out = io::Stdout{}.lock();
  fmt::write(out, args...);
}

template <class... T>
void println(const T&... args) {
  auto out = io::Stdout{}.lock();
  fmt::writeln(out, args...);
}

template <class... T>
auto eprint(const T&... args) -> void {
  fmt::write(io::Stdout{}.lock(), args...);
}

template <class... T>
auto eprintln(const T&... args) -> void {
  fmt::writeln(io::Stdout{}.lock(), args...);
}

}  // namespace rc::io
