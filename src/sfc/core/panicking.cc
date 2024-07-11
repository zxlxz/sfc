#include "panicking.h"

#include "sfc/backtrace/capture.h"

namespace sfc::panicking {

using backtrace::Backtrace;

struct PanicInfo {
  Location _loc;
  String _msg;
  Backtrace _backtrace;

 public:
  PanicInfo(Location loc, Str msg) : _loc{loc}, _msg{msg}, _backtrace{Backtrace::capture()} {}

  auto what() const -> const char* {
    static thread_local char buf[2048];

    auto len = 0U;
    auto print = [&](cstr_t fmt, const auto&... args) {
      const auto ret = __builtin_snprintf(buf + len, sizeof(buf) - len, fmt, args...);
      len += ret >= 0 ? static_cast<u32>(ret) : 0U;
    };

    print("%.*s\n", static_cast<u32>(_msg.len()), _msg.as_ptr());
    print("  > %s:%d\n", _loc.file, _loc.line);

    const auto frames = _backtrace.frames();
    auto frame_id = 0U;
    for (auto& frame : frames) {
      auto func = frame.func();
      if (!func.as_ptr()) {
        continue;
      }
      print("%2d: %.*s\n", frame_id, static_cast<u32>(func.len()), func.as_ptr());
      frame_id += 1;
    }

    return buf;
  }
};

void panic_str(Location loc, str::Str msg) {
  const auto err = PanicInfo{loc, msg};
  __builtin_printf("%s\n", err.what());
  throw Error{};
}

}  // namespace sfc::panicking
