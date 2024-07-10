#include "panicking.h"

#include "sfc/backtrace/capture.h"

namespace sfc::panicking {

using backtrace::Backtrace;
using backtrace::Frame;

struct PanicInfo {
  Location location;
  String message;
  Backtrace backtrace;

  auto print_to_buf(Slice<char> buf) {
    auto print = [buf](cstr_t fmts, auto... args) mutable {
      auto ret = __builtin_snprintf(buf._ptr, buf._len, fmts, args...);
      if (ret <= 0) {
        return;
      }

      const auto cnt = cmp::min(buf._len, static_cast<usize>(ret));
      buf._ptr += cnt;
      buf._len -= cnt;
    };

    print("%.*s\n", static_cast<u32>(message.len()), message.as_ptr());
    print("  > %s:%d\n", location.file, location.line);

    auto frame_id = 0U;
    for (auto& frame : backtrace.frames()) {
      const auto func = frame.func();
      if (!func) {
        continue;
      }
      print("[%2d] %.*s\n", frame_id, static_cast<u32>(func.len()), func.as_ptr());
      frame_id += 1;
    }
  }
};

static auto get_panic_info() -> PanicInfo& {
  thread_local PanicInfo res{};
  return res;
}

auto Error::what() const noexcept -> cstr_t {
  static thread_local char buf[4096];

  auto& info = get_panic_info();
  info.print_to_buf(buf);
  return buf;
}

void panic_str(Location loc, str::Str msg) {
  auto& panic_info = get_panic_info();

  panic_info.location = loc;
  panic_info.message = String{msg};
  panic_info.backtrace = Backtrace::capture();

  throw Error{};
}

}  // namespace sfc::panicking
