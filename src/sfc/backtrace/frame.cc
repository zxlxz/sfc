#include "frame.h"

#include "sfc/sys/backtrace.h"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::func() const -> String {
  static constexpr auto kMaxFuncLen = 256U;

  char func[kMaxFuncLen] = {};

  const auto info = sys_imp::resolve(_addr);
  sys_imp::cxx_demangle(info.func, func);

  return String::from_cstr(func);
}

auto capture() -> Vec<Frame> {
  static constexpr auto kMaxFrameCnt = 64U;

  void*      frame_buf[kMaxFrameCnt] = {};
  const auto frame_cnt = sys_imp::trace(frame_buf, kMaxFrameCnt);

  return Slice{frame_buf, frame_cnt}
      .iter()
      ->map([](auto frame_addr) { return Frame{frame_addr}; })
      ->template collect<Vec<Frame>>();
}

}  // namespace sfc::backtrace
