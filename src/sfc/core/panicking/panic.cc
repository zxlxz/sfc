#include "sfc/core/panicking.h"
#include "sfc/core/str.h"
#include "sfc/sys/backtrace.h"
#include "sfc/sys/io.h"

namespace sfc::panicking {

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
  char buf[256];
  auto buf_len = usize{0};

  auto append_to_buf = [&](Str s) {
    if (buf_len + s._len >= sizeof(buf)) {
      return;
    }
    ptr::copy_nonoverlapping(s._ptr, buf + buf_len, s._len);
    buf_len += s._len;
  };

  (void)(append_to_buf(args), ...);
  (void)(append_to_buf("\n"));

  sys::io::write(sys::io::stderr(), buf, buf_len);
}

static void dump_frame(u32 idx, void* ptr) noexcept {
  char fun_buf[256];

  const auto info = sys::backtrace::frame_info(ptr);
  const auto idx_str = idx2str(idx);
  const auto fun_len = sys::backtrace::cxx_demangle(info.func, fun_buf, sizeof(fun_buf));
  const auto fun_str = fun_len == 0 ? Str{info.func} : Str{fun_buf, fun_len};
  println(Str{" "}, idx_str, Str{" "}, fun_str);
}

void dump_frames() {
  void* frame_buf[64] = {};

  const auto frame_cnt = sys::backtrace::trace(frame_buf);
  for (auto idx = 0U; idx < frame_cnt; ++idx) {
    dump_frame(idx, frame_buf[idx]);
  }
}

void panic_str(Location loc, Str msg) noexcept {
  char line_buf[8] = {};
  const auto line_str = int2str(line_buf, static_cast<u32>(loc.line));

  println(msg);
  println(Str{" > "}, Str{loc.file}, Str{":"}, line_str);
  dump_frames();

  __builtin_trap();
}

}  // namespace sfc::panicking
