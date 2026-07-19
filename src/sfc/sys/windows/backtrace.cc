#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

#undef min
#undef max

#include "sfc/sys/windows/backtrace.h"

namespace sfc::sys::windows {

auto StackFrame::from_addr(const void* ptr) -> StackFrame {
  const auto proc = ::GetCurrentProcess();
  const auto addr = reinterpret_cast<ULONG_PTR>(ptr);

  auto res = StackFrame{};

  // file + line
  auto fl = ::IMAGEHLP_LINE64{};
  fl.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);

  auto fl_disp = 0UL;
  if (::SymGetLineFromAddr64(proc, addr, &fl_disp, &fl)) {
    res.file = fl.FileName;
  }

  // function name

  struct SymbolInfo : SYMBOL_INFO {
    char _NameBuf[sizeof(StackFrame::func)] = {};
  };
  auto sym = SymbolInfo{};
  sym.SizeOfStruct = sizeof(SYMBOL_INFO);
  sym.MaxNameLen = sizeof(sym._NameBuf);

  auto sym_disp = 0ULL;
  if (::SymFromAddr(proc, addr, &sym_disp, &sym)) {
    const auto ret = ::UnDecorateSymbolName(sym.Name, res.func, sizeof(res.func), UNDNAME_COMPLETE);
    if (ret == 0 || ret > sizeof(res.func)) {
      ::memcpy(res.func, sym.Name, sizeof(res.func) - 1);
      res.func[sizeof(res.func) - 1] = '\0';
    }
  }

  return res;
}

auto Backtrace::capture() -> Backtrace {
  static auto init = ::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);
  (void)init;

  auto res = Backtrace{};
  const auto ret = ::RtlCaptureStackBackTrace(0, kMaxFrame, res._frames, nullptr);
  if (ret >= 0 && ret <= int(kMaxFrame)) {
    res._len = ret;
  }
  return res;
}

auto Backtrace::len() const -> usize {
  return _len;
}

auto Backtrace::frame(usize idx) const -> StackFrame {
  if (idx >= _len) {
    return {};
  }

  const auto addr = _frames[idx];
  return StackFrame::from_addr(addr);
}

}  // namespace sfc::sys::windows
