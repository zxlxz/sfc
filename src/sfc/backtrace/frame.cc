#include "frame.h"

#include "sfc/sys/backtrace.h"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::func() const -> String {
  const auto info = sys_imp::get_frame(_addr);

  char buf[1024];
  const auto cnt = sys_imp::cxx_demangle(info.func, buf);
  if (cnt > 0 && cnt < sizeof(buf)) {
    return String::from(Str{buf, cnt});
  }

  return String::from(info.func);
}

auto capture() -> Vec<Frame> {
  void* buf[64] = {};
  const auto cnt = sys_imp::trace(buf);

  auto res =
      Slice{buf, cnt}.iter().map([](auto x) { return Frame{x}; }).template collect<Vec<Frame>>();
  return res;
}

}  // namespace sfc::backtrace
