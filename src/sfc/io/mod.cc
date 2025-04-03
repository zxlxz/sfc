#include "mod.h"

#include "sfc/sys/io.inl"

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
  const auto code = sys_imp::last_err();
  return Error::from_os_error(code);
}

auto Error::from_os_error(int code) -> Error {
  const auto kind = sys_imp::err_kind(code);
  return {kind, code};
}

auto Error::as_str() const -> Str {
  if (_code != 0) {
    return sys_imp::error_str(_code);
  }
  return reflect::enum_name(_kind);
}

}  // namespace sfc::io
