#include "capture.h"

#include "sfc/sys/backtrace.inl"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Backtrace::capture() noexcept -> Backtrace {
  auto res = Backtrace{};
  sys_imp::trace(res._frames);
  return res;
}

}  // namespace sfc::backtrace
