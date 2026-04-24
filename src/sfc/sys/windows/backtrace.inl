#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

struct Backtrace {
  static constexpr auto kMaxFrame = 64U;
  void* _frames[kMaxFrame];
  DWORD _count = 0;

 public:
  static auto capture() -> Backtrace {
    static const auto sym_init = ::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);
    if (!sym_init) {
      return {};
    }

    auto res = Backtrace{};
    res._count = ::RtlCaptureStackBackTrace(0, kMaxFrame, res._frames, nullptr);
    return res;
  }

  struct Frame;
  auto frame(DWORD idx) const -> Frame;
};

struct Backtrace::Frame {
  static constexpr auto kMaxName = 256U;
  PCHAR file = nullptr;
  DWORD line = 0;
  char func[kMaxName] = {};

 public:
  void parse_addr(const void* ptr) {
    const auto addr = reinterpret_cast<ULONG_PTR>(ptr);
    this->parse_file_line(addr);
    this->parse_func(addr);
  }

 private:
  void parse_file_line(ULONG_PTR addr) {
    auto fl = ::IMAGEHLP_LINE64{};
    fl.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);

    auto disp = 0UL;
    if (!::SymGetLineFromAddr64(::GetCurrentProcess(), addr, &disp, &fl)) {
      return;
    }
    this->file = fl.FileName;
    this->line = fl.LineNumber;
  }

  void parse_func(ULONG_PTR addr) {
    struct SymbolInfo : SYMBOL_INFO {
      char _NameBuf[kMaxName] = {};
    };
    auto sym = SymbolInfo{};
    sym.SizeOfStruct = sizeof(SYMBOL_INFO);
    sym.MaxNameLen = sizeof(sym._NameBuf);

    auto disp = 0ULL;
    if (!::SymFromAddr(::GetCurrentProcess(), addr, &disp, &sym)) {
      return;
    }

    const auto ret = ::UnDecorateSymbolName(sym.Name, this->func, kMaxName, UNDNAME_COMPLETE);
    if (ret <= 0 || ret > kMaxName) {
      ::memcpy(this->func, sym.Name, kMaxName);
    }
  }
};

inline auto Backtrace::frame(DWORD idx) const -> Frame {
  if (idx >= _count) {
    return {};
  }

  const auto addr = _frames[idx];
  auto frame = Frame{};
  frame.parse_addr(addr);
  return frame;
}

}  // namespace sfc::sys::windows
