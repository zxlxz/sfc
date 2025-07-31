#include "frame.h"

#include "sfc/sys/backtrace.h"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::to_str(Slice<char> buf) const -> Str {
  const auto info = sys_imp::get_frame(_addr);

  if (sys_imp::cxx_demangle(info.func, buf._ptr, static_cast<u32>(buf._len))) {
    return Str::from(buf._ptr);
  }

  return Str::from(info.func);
}

auto capture() -> Vec<Frame> {
  void* buf[64] = {};
  const auto cnt = sys_imp::trace(buf);

  auto res = Vec<Frame>{};
  for (auto i = 0U; i < cnt; ++i) {
    res.push(Frame{buf[i]});
  }

  return res;
}

}  // namespace sfc::backtrace
