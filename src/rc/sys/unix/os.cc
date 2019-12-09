#include "rc/sys/unix.inl"

namespace rc::sys::unix::os {

auto last_error() -> i32 {
  const auto ret = errno;
  return i32(ret);
}

}  // namespace rc::sys::unix::os
