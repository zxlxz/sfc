#pragma once

// windows
#include <Windows.h>

// windows-others
#include <DbgHelp.h>

#include "sfc/alloc.h"

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

static inline auto trace() -> Vec<void*> {
  static const auto kMaxFrames = 256U;

  static auto process = ::GetCurrentProcess();
  static auto sym_init = ::SymInitialize(process, nullptr, TRUE);
  if (!sym_init) {
    return {};
  }

  auto frames = Vec<void*>{};
  frames.reserve(kMaxFrames);
  const auto cnt = ::RtlCaptureStackBackTrace(0, kMaxFrames, frames.as_mut_ptr(), nullptr);
  if (cnt > 0) {
    frames.set_len(cnt);
  }

  return frames;
}

static inline auto resolve(void* addr) -> FrameInfo {
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

  auto line_info = ::IMAGEHLP_LINE64{};
  auto line_displayment = 0UL;
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
