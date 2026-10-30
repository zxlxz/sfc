#include "sfc/io/mod.h"
#include "sfc/sys/io.h"

namespace sfc::io {

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}

}  // namespace sfc::io
