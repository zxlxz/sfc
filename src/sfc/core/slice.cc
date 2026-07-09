#include "sfc/core/slice.h"
#include "sfc/core/io.h"
#include "sfc/core/result.h"

namespace sfc::slice {

template <class T>
auto Slice<T>::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (_len == 0 || buf._len == 0) {
    return io::Result<usize>{0UZ};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return io::Result<usize>{amt};
}

template auto Slice<u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
template auto Slice<const u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;

}  // namespace sfc::slice
