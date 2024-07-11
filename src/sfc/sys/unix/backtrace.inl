#pragma once

#include <dlfcn.h>
#include <execinfo.h>

#include "sfc/alloc.h"

namespace sfc::sys::backtrace {

struct Symbol {
  const char* file;
  const char* name;
};

static inline void trace(auto& frames) {
  static constexpr auto BUF_LEN = 64U;

  void* buf[BUF_LEN] = {nullptr};
  const auto cnt = ::backtrace(buf, BUF_LEN);
  if (cnt <= 0) {
    return;
  }

  frames.reserve(static_cast<u32>(cnt));
  for (auto i = 0; i < cnt; ++i) {
    frames.push({buf[i]});
  }
}

static inline auto resolve(void* addr) -> Symbol {
  auto info = ::Dl_info{};
  ::dladdr(addr, &info);

  auto res = Symbol{
      .file = info.dli_fname,
      .name = info.dli_sname,
  };
  return res;
}

}  // namespace sfc::sys::backtrace
