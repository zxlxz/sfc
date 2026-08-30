#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "sfc/sys/posix/backtrace.h"

namespace sfc::sys::posix {

static auto demangle(Slice<char> buf, const char* raw_name) -> bool {
  if (buf._ptr == nullptr || raw_name == nullptr) {
    return false;
  }

  auto status = int{0};
  auto out_len = buf._len;
  auto cxx_ptr = __cxxabiv1::__cxa_demangle(raw_name, buf._ptr, &out_len, &status);
  if (status == 0 && out_len < buf._len) {
    return true;
  }

  // cxx_ptr is allocate by __cxa_demangle and should be freed if not used
  if (cxx_ptr != buf._ptr) {
    ::free(cxx_ptr);
  }

  // fallback
  const auto managed_len = __builtin_strlen(raw_name);
  if (managed_len < buf._len) {
    __builtin_memcpy(buf._ptr, raw_name, managed_len);
    buf._ptr[managed_len] = '\0';
  }

  return false;
}

auto StackFrame::from_addr(const void* addr) -> StackFrame {
  auto dli = ::Dl_info{};
  if (!::dladdr(addr, &dli)) {
    return {};
  }

  auto res = StackFrame{};
  res.file = dli.dli_fname;
  (void)demangle(res.func, dli.dli_sname);

  return res;
}

auto Backtrace::capture() -> Backtrace {
  auto res = Backtrace{};
  const auto ret = ::backtrace(res._frames, kMaxFrame);
  if (ret >= 0 && ret <= int(kMaxFrame)) {
    res._len = num::cast_unsigned(ret);
  }
  return res;
}

auto Backtrace::len() const -> usize {
  return _len;
}

auto Backtrace::frame(usize idx) const -> StackFrame {
  if (idx >= _len) {
    return {};
  }

  const auto addr = _frames[idx];
  auto frame = StackFrame::from_addr(addr);
  return frame;
}

}  // namespace sfc::sys::posix
