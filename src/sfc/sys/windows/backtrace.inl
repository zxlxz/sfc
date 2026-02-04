#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::backtrace {

struct FrameInfo {
  static constexpr auto FUNC_SIZE = 256U;

  PCHAR file;
  DWORD line;
  char func[FUNC_SIZE];
};

struct SYMBOL_INFO_EX : SYMBOL_INFO {
  char _NameBuf[FrameInfo::FUNC_SIZE] = {};
};

static inline auto sym_init() -> bool {
  static const auto res = ::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);
  return res;
}

template <DWORD N>
static inline auto trace(void* (&frame_buf)[N]) -> SIZE_T {
  static constexpr auto SKIP_FRAME_CNT = 2U;

  if (!sym_init()) {
    return 0;
  }

  const auto cnt = ::RtlCaptureStackBackTrace(0, N, frame_buf, nullptr);
  if (cnt <= SKIP_FRAME_CNT) {
    return 0;
  }
  return cnt - SKIP_FRAME_CNT;
}

static inline auto frame_info(void* ptr) -> FrameInfo {
  if (!ptr || !sym_init()) {
    return {};
  }

  const auto addr = reinterpret_cast<ULONG_PTR>(ptr);

  auto sym_info = SYMBOL_INFO_EX{};
  sym_info.SizeOfStruct = sizeof(SYMBOL_INFO);
  sym_info.MaxNameLen = sizeof(sym_info._NameBuf);

  auto sym_displacement = 0ULL;
  if (!::SymFromAddr(::GetCurrentProcess(), addr, &sym_displacement, &sym_info)) {
    return {};
  }

  auto displacement = 0UL;
  auto line_info = ::IMAGEHLP_LINE64{};
  line_info.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);
  if (!::SymGetLineFromAddr64(::GetCurrentProcess(), addr, &displacement, &line_info)) {
    return {};
  }

  auto res = FrameInfo{
      .file = line_info.FileName,
      .line = line_info.LineNumber,
      .func = {},
  };
  ptr::copy_nonoverlapping(sym_info.Name, res.func, sizeof(sym_info._NameBuf));
  return res;
}

static inline auto cxx_demangle(const char in[], char buf[], DWORD buf_len) -> DWORD {
  const auto ret = ::UnDecorateSymbolName(in, buf, buf_len, UNDNAME_COMPLETE);
  if (ret <= 0 || ret > buf_len) {
    return 0;
  }
  return ret;
}

}  // namespace sfc::sys::.backtrace
