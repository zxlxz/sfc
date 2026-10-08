#include "sfc/io/mod.h"
#include "sfc/sys/io.h"

namespace sfc::io {

auto SeekFrom::Start(u64 offset) -> SeekFrom {
  const auto off = num::saturating_cast<i64>(offset);
  return SeekFrom{Kind::Start, off};
}

auto SeekFrom::Current(i64 offset) -> SeekFrom {
  return SeekFrom{Kind::Current, offset};
}

auto SeekFrom::End(i64 offset) -> SeekFrom {
  return SeekFrom{Kind::End, offset};
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys::os_error();
  const auto io_err = sys::io_error(os_err);
  return io_err;
}

}  // namespace sfc::io
