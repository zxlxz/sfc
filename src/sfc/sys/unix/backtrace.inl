#pragma once

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "sfc/alloc.h"

namespace sfc::sys::backtrace {

struct FrameInfo {
  static constexpr auto kMaxFuncLen = 256U;

  const char* file;
  unsigned    line;
  const char* func;
};

static inline auto trace(void* buf[], size_t buf_len) -> size_t {
  const auto cnt = ::backtrace(buf, buf_len);
  return cnt > 0 ? static_cast<size_t>(cnt) : 0;
}

static inline auto resolve(void* addr) -> FrameInfo {
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

static auto cxx_demangle(const char in[], char out_buf[], size_t buf_len) -> size_t {
  auto status = 0;
  __cxxabiv1::__cxa_demangle(in, out_buf, &buf_len, &status);
  return status == 0 ? buf_len : 0UL;
}

}  // namespace sfc::sys::backtrace
