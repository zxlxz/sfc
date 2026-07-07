#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#include "sfc/sys/unix/backtrace.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#include "sfc/sys/windows/backtrace.inl"
#endif

#include "sfc/core/panic.h"
#include "sfc/core/fmt.h"

namespace sfc::panic {

static void writeln(const auto&... args) {
  char buf[256];
  auto out = fmt::SBuf{buf};
  (void)(out.write_str(args), ...);
  (void)(out.write_str("\n"));

  const auto s = out.as_str();
  (void)(sys::Stderr().write(s.as_bytes()));
}

static auto write_buf(fmt::SBuf buf, const auto& args) -> Str {
  auto formatter = fmt::Formatter{buf};
  args.fmt(formatter);
  return buf.as_str();
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

void panic_imp(PanicInfo info) {
  char line_buf[8] = {};
  const auto line_str = panic::write_buf(line_buf, fmt::Args{"{02}", info._loc.line});
  panic::writeln(Str{"thread panicked at "}, Str::from_cstr(info._loc.file), Str{":"}, line_str);

  char msg_buf[1024];
  const auto msg_str = panic::write_buf(msg_buf, info._args);
  panic::writeln(msg_str);

  auto bt = sys::Backtrace::capture();
  for (auto idx = 0U; idx < bt._count; ++idx) {
    auto frame = bt.frame(idx);
    const auto idx_str = idx2str(idx);
    const auto fun_str = Str::from_cstr(frame.func);
    panic::writeln(Str{" "}, idx_str, Str{": "}, fun_str);
  }

  throw info;
}

[[noreturn]] void panic_fmt(fmt::Args args, SourceLoc loc) {
  const auto info = PanicInfo{args, loc};
  panic::panic_imp(info);
}

}  // namespace sfc::panic
