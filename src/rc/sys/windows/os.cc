#include "rc/sys/windows.inl"

namespace rc::sys::windows::os {

auto last_error() -> i32 {
  const auto ret = ::GetLastError();
  return i32(ret);
}

}  // namespace rc::sys::windows::os
