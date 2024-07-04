#include "sfc/io/error.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

auto to_str(ErrorKind kind) -> Str {
  switch (kind) {
    case ErrorKind::NotFound:          return "NotFound";
    case ErrorKind::PermissionDenied:  return "PermissionDenied";
    case ErrorKind::ConnectionRefused: return "ConnectionRefused";
    case ErrorKind::ConnectionReset:   return "ConnectionReset";
    case ErrorKind::ConnectionAborted: return "ConnectionAborted";
    case ErrorKind::NotConnected:      return "NotConnected";
    case ErrorKind::AddrInUse:         return "AddrInUse";
    case ErrorKind::AddrNotAvailable:  return "AddrNotAvailable";
    case ErrorKind::BrokenPipe:        return "BrokenPipe";
    case ErrorKind::AlreadyExists:     return "AlreadyExists";
    case ErrorKind::WouldBlock:        return "WouldBlock";
    case ErrorKind::InvalidInput:      return "InvalidInput";
    case ErrorKind::InvalidData:       return "InvalidData";
    case ErrorKind::TimedOut:          return "TimedOut";
    case ErrorKind::WriteZero:         return "WriteZero";
    case ErrorKind::UnexpectedEof:     return "UnexpectedEof";
    case ErrorKind::Interrupted:       return "Interrupted";
    default:                           return "Other";
  }
}

auto Error::last_os_error() -> Error {
  const auto code = sys_imp::get_err();
  const auto kind = sys_imp::kind_of<ErrorKind>(code);
  return Error{._kind = kind, ._code = code};
}

auto Error::from_os_error(int code) -> Error {
  const auto kind = sys_imp::kind_of<ErrorKind>(code);
  return Error{._kind = kind, ._code = code};
}

}  // namespace sfc::io
