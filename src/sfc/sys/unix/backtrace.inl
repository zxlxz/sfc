#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

struct Backtrace {
  static constexpr auto kMaxFrame = 64U;
  void* _frames[kMaxFrame];
  unsigned _count = 0;

 public:
  static auto capture() -> Backtrace {
    auto res = Backtrace{};
    const auto cnt = ::backtrace(res._frames, kMaxFrame);
    if (cnt >= 0) {
      res._count = static_cast<unsigned>(cnt);
    }
    return res;
  }

  struct Frame;
  auto frame(unsigned idx) const -> Frame;
};

struct Backtrace::Frame {
  static constexpr auto kMaxName = 256U;

  const char* file = nullptr;
  unsigned line = 0;
  char func[kMaxName] = {};

 public:
  void parse_addr(void* addr) {
    auto dli = ::Dl_info{};
    if (!::dladdr(addr, &dli)) {
      return;
    }

    this->file = dli.dli_fname;

    auto status = int{0};
    auto out_len = size_t{kMaxName};
    __cxxabiv1::__cxa_demangle(dli.dli_sname, this->func, &out_len, &status);
    if (status != 0 || out_len == 0 || out_len >= kMaxName) {
      ::strncpy(this->func, dli.dli_sname, kMaxName);
    }
  }
};

inline auto Backtrace::frame(unsigned idx) const -> Frame {
  if (idx >= _count) {
    return {};
  }

  const auto addr = _frames[idx];
  auto frame = Frame{};
  frame.parse_addr(addr);
  return frame;
}

}  // namespace sfc::sys::unix
