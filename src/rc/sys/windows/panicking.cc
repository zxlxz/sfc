#include "rc/sys/windows.inl"

#include "rc/io.h"

#pragma comment(lib, "DbgHelp.lib")

namespace rc::sys::windows::panicking {

struct Symbol {
  Str _file = u8"";
  usize _line = 0;

  struct {
    IMAGEHLP_SYMBOL64 _sym;
    char _sym_buf[1024];
  };

  auto func() const -> Str { return Str::from_cstr(_sym.Name); }

  static auto from_addr(const void* p) -> Symbol {
    if (p == nullptr) {
      return Symbol{};
    }

    const auto addr = u64(p);
    const auto pid = ::GetCurrentProcess();

    Symbol res;
    // func
    res._sym.MaxNameLength = sizeof(_sym_buf);
    if (::SymGetSymFromAddr64(pid, addr, nullptr, &res._sym) == FALSE) {
      res._sym.Name[0] = '\0';
    }

    // line
    auto line = ::_IMAGEHLP_LINE64{};
    line.SizeOfStruct = sizeof(line);

    auto disp = DWORD(0);
    if (::SymGetLineFromAddr64(pid, addr, &disp, &line) != FALSE) {
      res._file = Str::from_cstr(line.FileName);
      res._line = line.LineNumber;
    }

    return res;
  }
};

struct BackTrace {
  static constexpr u32 CAPACITY = 256;

  void* _buf[CAPACITY];
  u32 _len = 0;

  static auto capture() -> BackTrace {
    const HANDLE pid = GetCurrentProcess();
    (void)::SymInitialize(pid, nullptr, TRUE);
    (void)::SymSetOptions(SYMOPT_LOAD_LINES);

    BackTrace res;
    res._len = ::RtlCaptureStackBackTrace(0, CAPACITY, res._buf, nullptr);
    return res;
  }

  auto len() const noexcept -> u32 { return _len; }

  auto operator[](u32 idx) noexcept -> Symbol {
    if (idx >= _len) {
      return Symbol::from_addr(nullptr);
    }
    return Symbol::from_addr(_buf[idx]);
  }
};

auto panic(Str s) -> void {
  static thread_local auto sbuf = String{};
  sbuf.clear();

  auto sfmt = fmt::Formatter{sbuf};

  sfmt.write(u8"\x1b[31m[XX] {}\x1b[39m\n", s);

  auto bt = BackTrace::capture();
  bool panic_start = false;
  bool panic_end = false;
  for (u32 idx = 0; idx < bt.len(); ++idx) {
    const auto sym = bt[idx];
    const auto fun = sym.func();

    if (!panic_start) {
      if (fun.starts_with("rc::panicking")) {
        panic_start = true;
      }
      continue;
    }
    if (!panic_end) {
      if (!fun.starts_with("rc::panicking")) {
        panic_end = true;
      }
      continue;
    }

    if (fun == "main") break;
    sfmt.write(u8"   |-> \x1b[90m{}()\x1b[39m\n", fun);
  }
  io::Stderr::write(sbuf.as_bytes());
}

}  // namespace rc::sys::windows
