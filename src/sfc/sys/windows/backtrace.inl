#pragma once

// windows
#include <Windows.h>

// windows-others
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

namespace sfc::sys::backtrace {

struct FrameInfo {
  static constexpr auto kMaxFuncLen = 256U;

  char        func[kMaxFuncLen];
  const char* file;
  unsigned    line;
};

static inline auto sym_init() -> bool {
  static auto process = ::GetCurrentProcess();
  static auto sym_init = ::SymInitialize(process, nullptr, TRUE);
  return sym_init != FALSE;
}

static inline auto trace(void* frame_ptr[], SIZE_T frame_size) -> SIZE_T {
  static constexpr auto kSkipFrames = 2U;

  if (!sym_init()) {
    return 0;
  }

  const auto frame_cnt = ::RtlCaptureStackBackTrace(0, frame_size, frame_ptr, nullptr);
  return frame_cnt >= kSkipFrames ? frame_cnt - kSkipFrames : 0;
}

static inline auto resolve(void* addr) -> FrameInfo {
  if (addr == nullptr) {
    return {};
  }
  if (!sym_init()) {
    return {};
  }

  static auto process = ::GetCurrentProcess();

  struct : SYMBOL_INFO {
    char _NameBuf[sizeof(FrameInfo::func)] = {};
  } sym_info;
  sym_info.SizeOfStruct = sizeof(::SYMBOL_INFO);
  sym_info.MaxNameLen = sizeof(sym_info._NameBuf);

  auto sym_displacement = 0ULL;
  if (!::SymFromAddr(process, DWORD64(addr), &sym_displacement, &sym_info)) {
    return {};
  }

  auto line_displayment = 0UL;
  auto line_info = ::IMAGEHLP_LINE64{};
  line_info.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);
  if (::SymGetLineFromAddr64(process, DWORD64(addr), &line_displayment, &line_info) == 0) {
    return {};
  }

  auto res = FrameInfo{
      .file = line_info.FileName,
      .line = line_info.LineNumber,
  };
  __builtin_memcpy(res.func, sym_info._NameBuf, sizeof(sym_info._NameBuf));
  return res;
}

template <DWORD BUF_LEN>
static inline auto cxx_demangle(const char* in_buf, char (&out_buf)[BUF_LEN]) -> bool {
  const auto ret = ::UnDecorateSymbolName(in_buf, out_buf, BUF_LEN, UNDNAME_COMPLETE);
  return ret != 0;
}

}  // namespace sfc::sys::backtrace
