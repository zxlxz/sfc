#include "sfc/io/mod.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

auto Error::from_os_error(int code) -> Error {
  const auto kind = sys_imp::kind_of<ErrorKind>(code);
  return Error{.kind = kind, .code = code};
}

auto Error::last_os_error() -> Error {
  const auto os_err = sys_imp::get_err();
  return Error::from_os_error(os_err);
}

}  // namespace sfc::io
