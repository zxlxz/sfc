#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

#include "sfc/io/error.h"

namespace sfc::io {

auto Error::from_raw_os_error(ErrCode os_err) noexcept -> Error {
  const auto io_err = sys::io_error(os_err);
  return Error{io_err, os_err};
}

auto Error::last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return Error{io_err, os_err};
}
}  // namespace sfc::io
