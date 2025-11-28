#include "sfc/core/panicking.h"
#include "sfc/core/str.h"
#include "sfc/sys/backtrace.h"
#include "sfc/sys/io.h"

namespace sfc::panicking {

using str::Str;

struct Frame {
  void* _ptr = nullptr;

 public:
  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("<unknown>");
      return;
    }

    auto info = sys::backtrace::frame_info(_ptr);

    char func[256] = {};
    const auto len = sys::backtrace::cxx_demangle(info.func, func, sizeof(func));

    if (len >= 0) {
      f.write_str(Str{func, len});
    } else {
      f.write_str(Str::from_cstr(info.func));
    }
  }
};

struct Backtrace {
  static constexpr auto MAX_FRAME_CNT = 64U;

  void* _frame[MAX_FRAME_CNT];
  usize _frame_cnt = 0;

 public:
  static auto capture() -> Backtrace {
    auto res = Backtrace{};
    res._frame_cnt = sys::backtrace::trace(res._frame);
    return res;
  }

  auto len() const -> usize {
    return _frame_cnt;
  }

  auto operator[](usize idx) const -> Frame {
    if (idx >= _frame_cnt) {
      return Frame{nullptr};
    }
    return Frame{_frame[idx]};
  }
};

struct FixedBuf {
  char _buf[1024];
  usize _len = 0;

 public:
  void write_str(Str s) {
    if (s._len == 0 || _len + s._len > sizeof(_buf)) {
      return;
    }
    ptr::copy_nonoverlapping(s._ptr, _buf + _len, s._len);
    _len += s._len;
  }
};

static void dump_fmt(const auto&... args) {
  auto buf = FixedBuf{};
  fmt::write(buf, args...);

  sys::io::write(sys::io::stderr(), buf._buf, buf._len);
}

void panic_imp(Location loc, Str msg) {
  panicking::dump_fmt("{}\n", msg);
  panicking::dump_fmt(" >: {}:{:02}\n", Str::from_cstr(loc.file), loc.line);

  auto bt = Backtrace::capture();
  for (auto i = 0U; i < bt.len(); ++i) {
    panicking::dump_fmt("  {:02}: {}\n", i, bt[i]);
  }

  __builtin_trap();
}

}  // namespace sfc::panicking
