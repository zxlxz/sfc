#include "frame.h"

#include "sfc/sys/backtrace.h"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::func() const -> String {
  const auto info = sys_imp::resolve(_addr);
  return String::from(Str::from_cstr(info.func));
}

auto Backtrace::capture() -> Backtrace {
  auto res = Backtrace{};
  res._frames.reserve(64);

  const auto frame_cnt = sys_imp::trace(res._frames.as_mut_ptr(), res._frames.capacity());
  res._frames.set_len(frame_cnt);

  return res;
}

}  // namespace sfc::backtrace
