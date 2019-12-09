#include "rc/sys/unix.inl"

namespace rc::sys::unix {

auto FileDesc::drop() -> void {
  if (_raw == -1) return;
  ::close(_raw);
  _raw = -1;
}

}  // namespace rc::sys::unix
