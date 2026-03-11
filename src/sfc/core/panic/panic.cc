#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#include "sfc/sys/unix/backtrace.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#include "sfc/sys/windows/backtrace.inl"
#endif
#define _SFC_SYS_IO_

#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"

namespace sfc::panic {

template <u32 N>
static auto int2str(char (&buf)[N], u32 val) -> Str {
  auto idx = N;
  while (idx != 0) {
    buf[--idx] = static_cast<char>('0' + val % 10);
    val /= 10;
    if (val == 0) {
      break;
    }
  }
  return Str{buf + idx, N - idx};
}

static auto idx2str(u32 idx) -> Str {
  static constexpr auto digits =
      " 0 1 2 3 4 5 6 7 8 9"
      "10111213141516171819"
      "20212223242526272829"
      "30313233343536373839"
      "40414243444546474849"
      "50515253545556575859"
      "60616263646566676869"
      "70717273747576777879"
      "80818283848586878889"
      "90919293949596979899";

  const auto idx_ptr = idx <= 99 ? digits + 2 * idx : "??";
  return Str{idx_ptr, 2};
}

static void println(const auto&... args) noexcept {
  auto buf = fmt::FixedBuf<256>{};
  (void)(buf.write_str(args), ...);
  (void)(buf.write_str("\n"));

  auto stdout = sys::stdout();
  stdout.write(buf._buf, buf._len);
}

void panic_imp(Location loc, const char* msg, usize len) {
  char line_buf[8] = {};
  const auto line_str = int2str(line_buf, static_cast<u32>(loc.line));

  println(Str{msg, len});
  println(" > ", loc.file, ":", line_str);

  auto bt = sys::Backtrace::capture();
  for (auto idx = 0U; idx < bt._count; ++idx) {
    auto frame = bt.frame(idx);
    const auto idx_str = idx2str(idx);
    const auto fun_str = frame.func;
    println(" ", idx_str, " ", fun_str);
  }

  throw Error{loc};
}

}  // namespace sfc::panic
