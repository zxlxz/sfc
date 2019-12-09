#include "rc/sys/unix.inl"

namespace rc::sys::unix::term {

auto columns() -> usize {
  struct ::winsize console_info {};

  const auto ret = ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &console_info);

  if (ret == -1) {
    return 120u;
  }

  return usize(console_info.ws_col);
}

}  // namespace rc::sys::unix::term
