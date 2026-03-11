#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

struct Backtrace {
  static constexpr auto MAX_FRAME = 64U;
  void* _frames[MAX_FRAME];
  DWORD _count = 0;

 public:
  static auto capture() -> Backtrace {
    static const auto sym_init = ::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);
    if (!sym_init) {
      return {};
    }

    auto res = Backtrace{};
    res._count = ::RtlCaptureStackBackTrace(0, MAX_FRAME, res._frames, nullptr);
    return res;
  }

  struct LineInfo {
    PCHAR file = nullptr;
    DWORD line = 0;

    static auto from_addr(ULONG_PTR addr) -> LineInfo {
      auto fl = ::IMAGEHLP_LINE64{};
      fl.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);

      auto disp = 0UL;
      if (!::SymGetLineFromAddr64(::GetCurrentProcess(), addr, &disp, &fl)) {
        return {};
      }
      return {fl.FileName, fl.LineNumber};
    }
  };

  struct FuncInfo {
    static constexpr auto MAX_NAME = 256U;
    char func[MAX_NAME] = {};

    static auto from_addr(ULONG_PTR addr) -> FuncInfo {
      struct SymbolInfo : SYMBOL_INFO {
        char _NameBuf[MAX_NAME] = {};
      };
      auto sym = SymbolInfo{};
      sym.SizeOfStruct = sizeof(SYMBOL_INFO);
      sym.MaxNameLen = sizeof(sym._NameBuf);

      auto disp = 0ULL;
      if (!::SymFromAddr(::GetCurrentProcess(), addr, &disp, &sym)) {
        return {};
      }

      auto res = FuncInfo{};
      const auto ret = ::UnDecorateSymbolName(sym.Name, res.func, MAX_NAME, UNDNAME_COMPLETE);
      if (ret <= 0 || ret > MAX_NAME) {
        ::memcpy(res.func, sym.Name, MAX_NAME);
      }
      return res;
    }
  };

  struct FrameInfo : LineInfo, FuncInfo {};

  auto frame(DWORD idx) const -> FrameInfo {
    if (idx > _count) {
      return {};
    }

    const auto addr = reinterpret_cast<uintptr_t>(_frames[idx]);
    const auto line_info = LineInfo::from_addr(addr);
    const auto func_info = FuncInfo::from_addr(addr);
    return {line_info, func_info};
  }
};

}  // namespace sfc::sys::windows
