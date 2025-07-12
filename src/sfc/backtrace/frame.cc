#include "frame.h"

#include "sfc/sys/backtrace.h"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::func() const -> String {
  const auto info     = sys_imp::resolve(_addr);
  const auto raw_func = Str::from_cstr(info.func);

  auto res = String{};
  res.reserve(raw_func.len() * 2 + 32);
  const auto nbytes = sys_imp::cxx_demangle(info.func, res.as_mut_ptr(), res.capacity());
  res.set_len(nbytes);

  return res;
}

auto capture() -> Vec<Frame> {
  static constexpr auto kMaxFrameCnt = 64U;

  void*      frame_buf[kMaxFrameCnt] = {};
  const auto frame_cnt               = sys_imp::trace(frame_buf, kMaxFrameCnt);

  return Slice{frame_buf, frame_cnt}
      .iter()
      .map([](auto frame_addr) { return Frame{frame_addr}; })
      .template collect<Vec<Frame>>();
}

}  // namespace sfc::backtrace
