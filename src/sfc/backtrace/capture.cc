#include "capture.h"

#include "sfc/sys/backtrace.inl"

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

Backtrace::Backtrace() noexcept = default;

Backtrace::Backtrace(Vec<Frame> frames) noexcept : _frames{mem::move(frames)} {}

Backtrace::Backtrace(Backtrace&&) noexcept = default;

Backtrace::~Backtrace() = default;

Backtrace& Backtrace::operator=(Backtrace&&) noexcept = default;

auto Backtrace::frames() const -> Slice<const Frame> {
  return _frames.as_slice();
}

auto Backtrace::capture() noexcept -> Backtrace {
  auto res = Backtrace{};
  sys_imp::trace(res._frames);
  return res;
}

}  // namespace sfc::backtrace
