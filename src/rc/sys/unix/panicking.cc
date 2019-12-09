#include "rc/sys/unix.inl"

#include "rc/io.h"

namespace rc::sys::unix::panicking {

struct Symbol {
  void* _addr = nullptr;
  const char* _desc = nullptr;
  char** _buff = nullptr;

  explicit Symbol(void* addr) : _addr{addr} {
    if (_addr = nullptr) return;

    _buff = ::backtrace_symbols(&addr, 1);
    if (_buff != nullptr) {
      _desc = _buff[0];
    }
  }

  ~Symbol() {
    if (_buff == nullptr) return;
    ::free(_buff);
  }

  auto func() const -> Str {
    const auto desc = Str::from_cstr(_desc);

    auto arr = desc.split(u8' ');
    auto idx = arr.next();
    auto lib = arr.next();
    auto addr = arr.next();
    auto func = arr.next();
    return desc;
    // return rc::move(idx).unwrap_or(u8"");
  }
};

struct Backtrace {
  static constexpr usize CAPACITY = 256;

  void* _stacks[CAPACITY];
  usize _len = 0;

  static auto capture() -> Backtrace {
    Backtrace res;
    res._len = ::backtrace(res._stacks, CAPACITY);
    return res;
  }

  auto len() const noexcept -> u32 { return _len; }

  auto operator[](usize idx) noexcept -> Symbol {
    if (idx >= _len) return Symbol{nullptr};
    return Symbol{_stacks[idx]};
  }
};

auto panic(Str s) -> void {
  thread_local auto sbuf = string::String{};
  sbuf.clear();

  auto sfmt = fmt::Formatter{sbuf};
  sfmt.write(u8"\x1b[31m[XX] {}\x1b[39m\n", s);

  auto bt = Backtrace::capture();
  for (usize idx = 0u; idx < bt.len(); ++idx) {
    const auto sym = bt[idx];
    const auto fun = sym.func();
    if (fun == "main") break;
    sfmt.write(u8"   |-> \x1b[90m{}()\x1b[39m\n", fun);
  }
  io::Stderr::write(sbuf.as_bytes());
}

}  // namespace rc::sys::unix::panicking
