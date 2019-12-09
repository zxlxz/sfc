#include "rc/sys/windows.inl"

namespace rc::sys::windows::term {

auto columns() -> usize {
  auto ifo = ::_CONSOLE_SCREEN_BUFFER_INFO{};

  auto fid = ::GetStdHandle(STD_OUTPUT_HANDLE);
  auto ret = ::GetConsoleScreenBufferInfo(fid, &ifo);

  if (!ret) {
    return 120u;
  }

  return usize(ifo.dwSize.X);
}

}  // namespace rc::sys::windows::term
