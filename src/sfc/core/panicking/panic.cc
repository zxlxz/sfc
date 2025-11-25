#include "sfc/core/panicking.h"
#include "sfc/core/str.h"
#include "sfc/sys/backtrace.h"
#include "sfc/sys/io.h"

namespace sfc::panicking {

using str::Str;

static void panic_dump(const auto&... args) {
  char buf[256];
  auto buf_len = usize{0};

  auto buf_append = [&](Str s) {
    if (buf_len + s._len < sizeof(buf)) {
      ptr::copy_nonoverlapping(s._ptr, buf + buf_len, s._len);
      buf_len += s._len;
    }
  };
  (void)(buf_append(args), ...);

  sys::io::write(sys::io::stderr(), buf, buf_len);
}

static void panic_dump_frames() {
  void* frame_buf[64] = {};
  const auto frame_cnt = sys::backtrace::trace(frame_buf);

  for (auto idx = 0U; idx < frame_cnt; ++idx) {
    char index_buf[] = "  : ";
    index_buf[0] = idx < 10 ? ' ' : char('0' + idx / 10);
    index_buf[1] = char('0' + idx % 10);
    auto frame_idx = Str{index_buf, sizeof(index_buf) - 1};

    auto frame_ptr = frame_buf[idx];
    auto frame_info = sys::backtrace::frame_info(frame_ptr);
    auto frame_func = Str::from_cstr(frame_info.func);
    char func_buf[256] = {};
    if (auto len = sys::backtrace::cxx_demangle(frame_info.func, func_buf, sizeof(func_buf))) {
      frame_func = Str{func_buf, len};
    }

    panic_dump(frame_idx, frame_func, Str{"\n"});
  }
}

void panic_str(Str msg) {
  panic_dump(msg, Str{"\n"});
  panic_dump_frames();
  throw PanicErr{};
}

}  // namespace sfc::panicking
