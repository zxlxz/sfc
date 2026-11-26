#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"
#include "sfc/sys/io.h"
#include "sfc/sys/backtrace.h"

namespace sfc::panic {

void SourceLoc::fmt(fmt::Formatter& f) const {
  f.write_fmt("{}:{}", Str::from_cstr(file), line);
}

void panic_imp(PanicInfo info) {
  constexpr usize kBufSize = 4096;

  struct Buf {
    u8 _buf[kBufSize];
    usize _len = 0;

   public:
    void write_str(Str s) noexcept {
      const auto n = cmp::min(s._len, kBufSize - _len);
      __builtin_memcpy(_buf + _len, s._ptr, n);
      _len += n;
    }
  };

  auto buf = Buf{};
  auto fmter = fmt::Formatter{buf};
  fmter.write_fmt("thread panicked at {}\n{}\n", info._loc, info._args);

  auto bt = sys::Backtrace::capture();
  for (auto idx : Range{bt.len()}) {
    auto frame = bt.frame(idx);
    fmter.write_fmt(" {2}: {}\n", idx, frame.func);
  }

  auto sys_out = sys::Stderr();
  (void)sys_out.write({buf._buf, buf._len});

  throw info;
}

}  // namespace sfc::panic
