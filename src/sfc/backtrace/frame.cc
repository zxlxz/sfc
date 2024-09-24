#include "frame.h"

#include "sfc/sys/backtrace.inl"

extern "C" {
char* __cxa_demangle(const char* raw_name, char* buf, sfc::usize* len, int* ret);
}

namespace sfc::backtrace {

namespace sys_imp = sys::backtrace;

static auto cxx_demangle(Str raw) -> String {
  static constexpr auto MAX_LEN = 1024;

  if (raw.is_empty()) {
    return {};
  }
  if (raw.len() >= MAX_LEN / 3 || !raw.starts_with("_Z")) {
    return String{raw};
  }

  char buf[MAX_LEN];
  auto len = sizeof(buf);
  auto ret = 0;
  auto res = __cxa_demangle(raw.as_ptr(), buf, &len, &ret);
  return String{res ?: raw};
}

Frame::Frame(void* addr) : _addr{addr} {}

Frame::~Frame() {}

auto Frame::func() const -> String {
  auto sym = sys_imp::resolve(_addr);
  auto res = cxx_demangle(sym.name);
  return res;
}

}  // namespace sfc::backtrace
