#include "frame.h"

#include "sfc/sys/backtrace.inl"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

auto Frame::func() const -> String {
  const auto info = sys_imp::resolve(_addr);
  return String::from(Str::from_cstr(info.func));
}

auto capture() -> Vec<Frame> {
  auto frames = sys_imp::trace();

  auto res = Vec<Frame>{};
  res.reserve(frames.len());
  for (auto frame_ptr : frames.as_slice()) {
    res.push({frame_ptr});
  }

  return res;
}

}  // namespace sfc::backtrace
