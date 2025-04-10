#pragma once

// windows
#include <Windows.h>

// windows-others
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

namespace sfc::sys::backtrace {

struct FrameInfo {
  const char* func;
  const char* file;
  unsigned    line;
};

static inline auto cxx_demangle(const char* in_buf, char* out_buf, DWORD out_len) {
  const auto ret = ::UnDecorateSymbolName(in_buf, out_buf, out_len, UNDNAME_COMPLETE);
  return ret != 0;
}

static inline auto sym_init() -> bool {
  static auto process = ::GetCurrentProcess();
  static auto sym_init = ::SymInitialize(process, nullptr, TRUE);
  return sym_init != FALSE;
}

static inline auto trace(void* frame_ptr[], SIZE_T frame_size) -> SIZE_T {
  if (!sym_init()) {
    return 0;
  }

  const auto frame_cnt = ::RtlCaptureStackBackTrace(0, frame_size, frame_ptr, nullptr);
  return frame_cnt;
}

static inline auto resolve(void* addr) -> FrameInfo {
  if (addr == nullptr) {
    return {};
  }
  if (!sym_init()) {
    return {};
  }

  static auto process = ::GetCurrentProcess();

  auto sym_info = ::SYMBOL_INFO_PACKAGE{};
  auto sym_displacement = 0ULL;

  if (::SymFromAddr(process, DWORD64(addr), &sym_displacement, &sym_info.si)) {
    return {};
  }

  static thread_local char func_name[MAX_SYM_NAME];
  if (!cxx_demangle(sym_info.si.Name, func_name, sizeof(sym_info.name))) {
    ::strncpy_s(func_name, sym_info.si.Name, sizeof(func_name));
  }

  auto line_displayment = 0UL;
  auto line_info = ::IMAGEHLP_LINE64{};
  line_info.SizeOfStruct = sizeof(::IMAGEHLP_LINE64);
  if (::SymGetLineFromAddr64(process, DWORD64(addr), &line_displayment, &line_info) == 0) {
    return {};
  }

  const auto res = FrameInfo{
      .func = func_name,
      .file = line_info.FileName,
      .line = line_info.LineNumber,
  };
  return res;
}

}  // namespace sfc::sys::backtrace
