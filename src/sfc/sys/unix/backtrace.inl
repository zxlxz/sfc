#pragma once

#include <dlfcn.h>
#include <execinfo.h>
#include <cxxabi.h>

#include "sfc/alloc.h"

namespace sfc::sys::backtrace {

struct FrameInfo {
  const char* func;
};

static auto cxx_demangle(const char* in, char* out_buf, size_t out_len) -> bool {
  auto status = 0;
  ::__cxa_demangle(in, out_buf, &out_len, &status);
  return status == 0;
}

static inline auto trace() -> Vec<void*> {
  static const auto kMaxFrames = 128U;

  auto frames = Vec<void*>{};
  frames.reserve(kMaxFrames);
  const auto cnt = ::backtrace(frames.as_mut_ptr(), kMaxFrames);
  if (cnt > 0) {
    frames.set_len(uint16_t(cnt));
  }

  return frames;
}

static inline auto resolve(void* addr) -> FrameInfo {
  auto dl_info = ::Dl_info{};
  if (!::dladdr(addr, &dl_info)) {
    return {};
  }

  static thread_local char func_name[2048];
  if (!cxx_demangle(dl_info.dli_sname, func_name, sizeof(func_name))) {
    ::strncpy(func_name, dl_info.dli_sname, sizeof(func_name));
  }

  const auto res = FrameInfo {
      .func = func_name
  };
  return res;
}

}  // namespace sfc::sys::backtrace
