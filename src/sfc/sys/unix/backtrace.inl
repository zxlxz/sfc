#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

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

static inline auto frame_info(void* addr) -> FrameInfo {
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

static auto cxx_demangle(const char in[], char buf[], size_t buf_len) -> size_t {
  auto status = 0;
  auto out_len = buf_len;
  __cxxabiv1::__cxa_demangle(in, buf, &out_len, &status);
  if (status != 0 || out_len == 0 || out_len >= buf_len) {
    return 0;
  }
  return out_len;
}

}  // namespace sfc::sys::backtrace
#endif
