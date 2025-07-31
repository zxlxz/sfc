#pragma once

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "sfc/alloc.h"

namespace sfc::sys::backtrace {

struct FrameInfo {
  const char* file;
  unsigned line;
  const char* func;
};

template <size_t N>
static inline auto trace(void* (&buf)[N]) -> size_t {
  const auto cnt = ::backtrace(buf, N);
  return cnt > 0 ? cnt : 0U;
}

static inline auto get_frame(void* addr) -> FrameInfo {
  auto dl_info = ::Dl_info{};
  if (!::dladdr(addr, &dl_info)) {
    return {};
  }

  auto res = FrameInfo{
      .file = dl_info.dli_fname,
      .line = 0,
      .func = dl_info.dli_sname,
  };

  return res;
}

static auto cxx_demangle(const char in[], char buf[], size_t buf_len) -> bool {
  auto status = 0;
  __cxxabiv1::__cxa_demangle(in, buf, &buf_len, &status);
  return status == 0;
}

}  // namespace sfc::sys::backtrace
