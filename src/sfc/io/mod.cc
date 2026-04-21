#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/io.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/io.inl"
#endif

namespace sfc::io {

auto from_raw_os_error(i32 err) noexcept -> Error{
  const auto io_err = sys::io_error(err);
  return io_err;
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}
}  // namespace sfc::io
