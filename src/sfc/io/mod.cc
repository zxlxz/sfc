#include "mod.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

SFC_ENUM(ErrorKind,
         NotFound,
         PermissionDenied,
         ConnectionRefused,
         ConnectionReset,
         ConnectionAborted,
         NotConnected,
         AddrInUse,
         AddrNotAvailable,
         BrokenPipe,
         AlreadyExists,
         WouldBlock,
         InvalidInput,
         InvalidData,
         TimedOut,
         WriteZero,
         UnexpectedEof,
         Interrupted,
         Other);

auto Error::last_os_error() -> Error {
  const auto imp = sys_imp::Error::last();
  return Error{imp.kind(), imp.code()};
}

auto Error::from_os_error(int code) -> Error {
  const auto imp = sys_imp::Error{sys_imp::error_t(code)};
  return Error{imp.kind(), code};
}

auto Error::as_str() const -> Str {
  return reflect::enum_name(_kind);
}

}  // namespace sfc::io
