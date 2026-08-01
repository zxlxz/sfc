#include "sfc/core/io.h"
#include "sfc/core/str.h"
#include "sfc/core/slice.h"
#include "sfc/core/result.h"

namespace sfc::slice {

template <class T>
auto Slice<T>::read(Slice<u8> buf) noexcept -> io::Result<usize>
  requires(trait::same_<const T, const u8>)
{
  if (buf._len == 0 || _len == 0) {
    return usize{0};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return usize{amt};
}

template <class T>
auto Slice<T>::write(Slice<const u8> buf) noexcept -> io::Result<usize>
  requires(trait::same_<T, u8>)
{
  if (buf._len == 0 || _len == 0) {
    return usize{0};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(buf._ptr, _ptr, amt);
  _ptr += amt;
  _len -= amt;

  return usize{amt};
}

template <class T>
auto Slice<T>::write_str(Str buf) noexcept -> io::Result<>
  requires(trait::same_<T, char>)
{
  if (buf._len == 0 || _len == 0) {
    return Ok();
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(buf._ptr, _ptr, amt);
  _ptr += amt;
  _len -= amt;
  return Ok();
}

template auto Slice<u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
template auto Slice<const u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;

template auto Slice<u8>::write(Slice<const u8> buf) noexcept -> io::Result<usize>;

template auto Slice<char>::write_str(Str s) noexcept -> io::Result<>;

}  // namespace sfc::slice
