#include "backtrace.h"

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

namespace sfc::backtrace {

static auto cxx_demangle(Str raw) -> String {
  static constexpr auto MAX_LEN = 1024;

  if (raw.is_empty()) {
    return {};
  }
  if (raw.len() >= MAX_LEN / 3 || !raw.starts_with("_Z")) {
    return String{raw};
  }

  auto ret = 0;
  char buf[MAX_LEN];
  auto len = sizeof(buf);
  auto fun = abi::__cxa_demangle(raw.as_ptr(), buf, &len, &ret);
  auto res = String{fun ? fun : raw};
  return res;
}

Frame::Frame(void* addr) : _addr{addr} {}

Frame::~Frame() {}

auto Frame::func() const -> String {
  Dl_info info;
  ::dladdr(_addr, &info);

  auto res = cxx_demangle(info.dli_sname);
  return res;
}

Backtrace::Backtrace() noexcept = default;

Backtrace::Backtrace(Vec<Frame> frames) noexcept : _frames{mem::move(frames)} {}

Backtrace::Backtrace(Backtrace&&) noexcept = default;

Backtrace::~Backtrace() = default;

Backtrace& Backtrace::operator=(Backtrace&&) noexcept = default;

auto Backtrace::frames() const -> Slice<const Frame> {
  return _frames.as_slice();
}

auto Backtrace::capture() noexcept -> Backtrace {
  static constexpr auto BUF_LEN = 64U;

  void* buf[BUF_LEN] = {nullptr};

  const auto cnt = ::backtrace(buf, BUF_LEN);

  auto frames = Vec<Frame>{};
  for (auto i = 0; i < cnt; ++i) {
    frames.push(Frame{buf[i]});
  }
  return Backtrace{mem::move(frames)};
}

}  // namespace sfc::backtrace
