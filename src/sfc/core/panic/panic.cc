#include "sfc/core/panic.h"

#include "sfc/core/fmt.h"
#include "sfc/sys/backtrace.h"
#include "sfc/sys/io.h"

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
  auto buf = fmt::FixedBuf<256U>{};
  (void)(buf.write_str(Str{args}), ...);
  (void)(buf.write_str(Str{"\n"}));
  sys::io::write(sys::io::stderr(), buf._buf, buf._len);
}

static void dump_frame(u32 idx, void* ptr) noexcept {
  char fun_buf[256];

  const auto info = sys::backtrace::frame_info(ptr);
  const auto raw_fun = Str::from_cstr(info.func);

  const auto demangle_len = sys::backtrace::cxx_demangle(info.func, fun_buf, sizeof(fun_buf));
  const auto demangle_fun = Str{fun_buf, demangle_len};

  const auto idx_str = idx2str(idx);
  const auto fun_str = demangle_len == 0 ? raw_fun : demangle_fun;
  println(Str{" "}, idx_str, Str{" "}, fun_str);
}

void dump_frames() {
  void* frame_buf[64] = {};

  const auto frame_cnt = sys::backtrace::trace(frame_buf);
  for (auto idx = 0U; idx < frame_cnt; ++idx) {
    dump_frame(idx, frame_buf[idx]);
  }
}

void panic_imp(Location loc, const char* msg, usize len) noexcept {
  char line_buf[8] = {};
  const auto line_str = int2str(line_buf, static_cast<u32>(loc.line));

  println(Str{msg, len});
  println(Str{" > "}, Str::from_cstr(loc.file), Str{":"}, line_str);
  dump_frames();

  __builtin_trap();
}

}  // namespace sfc::panic
