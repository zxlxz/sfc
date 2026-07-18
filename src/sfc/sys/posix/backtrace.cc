#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>

#include "sfc/sys/posix/backtrace.h"

namespace sfc::sys::posix {

auto StackFrame::from_addr(const void* addr) -> StackFrame {
  auto dli = ::Dl_info{};
  if (!::dladdr(addr, &dli)) {
    return {};
  }

  auto res = StackFrame{};
  res.file = dli.dli_fname;

  auto out_len = sizeof(res.func);

  auto status = int{0};
  __cxxabiv1::__cxa_demangle(dli.dli_sname, res.func, &out_len, &status);
  if (status != 0 || out_len == 0 || out_len >= sizeof(res.func)) {
    __builtin_strncpy(res.func, dli.dli_sname, sizeof(res.func) - 1);
    res.func[sizeof(res.func) - 1] = '\0';
  }
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
