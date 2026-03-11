#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

struct Backtrace {
  static constexpr auto MAX_FRAME = 64U;
  void* _frames[MAX_FRAME];
  unsigned _count = 0;

 public:
  static auto capture() -> Backtrace {
    auto res = Backtrace{};
    const auto cnt = ::backtrace(res._frames, MAX_FRAME);
    if (cnt >= 0) {
      res._count = static_cast<unsigned>(cnt);
    }
    return res;
  }

  struct FrameInfo {
    static constexpr size_t MAX_NAME = 256U;
    const char* file;
    unsigned line;
    char func[MAX_NAME];

    static auto from_addr(void* addr) -> FrameInfo {
      auto dli = ::Dl_info{};
      if (!::dladdr(addr, &dli)) {
        return {};
      }

      auto res = FrameInfo{dli.dli_fname, 0, {}};

      auto status = 0;
      auto out_len = MAX_NAME;
      __cxxabiv1::__cxa_demangle(dli.dli_sname, res.func, &out_len, &status);
      if (status != 0 || out_len == 0 || out_len >= MAX_NAME) {
        ::strncpy(res.func, dli.dli_sname, sizeof(res.func));
      }

      return res;
    }
  };

  auto frame(unsigned idx) const -> FrameInfo {
    if (idx >= _count) {
      return {};
    }

    const auto addr = _frames[idx];
    return FrameInfo::from_addr(addr);
  }
};

}  // namespace sfc::sys::unix
