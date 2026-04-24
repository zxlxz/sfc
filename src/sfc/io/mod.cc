#include "sfc/io/mod.h"
#include "sfc/alloc/vec.h"

namespace sfc {

namespace slice {

template <class T>
auto Slice<T>::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (_len == 0 || buf._len == 0) {
    return usize{0};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return amt;
}

template auto Slice<u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
template auto Slice<const u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
}  // namespace slice

namespace vec {

template <class T, class A>
auto Vec<T, A>::write(Slice<const u8> buf) -> io::Result<usize> {
  this->extend_from_slice(buf);
  return buf.len();
}

template auto Vec<u8>::write(Slice<const u8> buf) -> io::Result<usize>;
}  // namespace vec

}  // namespace sfc
