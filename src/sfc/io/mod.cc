#include "sfc/io/mod.h"

#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

auto Error::as_str() const -> str::Str {
  switch (kind) {
    case ErrorKind::NotFound:               return "entity not found";
    case ErrorKind::PermissionDenied:       return "permission denied";
    case ErrorKind::ConnectionRefused:      return "connection refused";
    case ErrorKind::ConnectionReset:        return "connection reset";
    case ErrorKind::HostUnreachable:        return "host unreachable";
    case ErrorKind::NetworkUnreachable:     return "network unreachable";
    case ErrorKind::ConnectionAborted:      return "connection aborted";
    case ErrorKind::NotConnected:           return "not connected";
    case ErrorKind::AddrInUse:              return "address in use";
    case ErrorKind::AddrNotAvailable:       return "address not available";
    case ErrorKind::NetworkDown:            return "network down";
    case ErrorKind::BrokenPipe:             return "broken pipe";
    case ErrorKind::AlreadyExists:          return "entity already exists";
    case ErrorKind::WouldBlock:             return "operation would block";
    case ErrorKind::NotADirectory:          return "not a directory";
    case ErrorKind::IsADirectory:           return "is a directory";
    case ErrorKind::DirectoryNotEmpty:      return "directory not empty";
    case ErrorKind::ReadOnlyFilesystem:     return "read-only filesystem";
    case ErrorKind::FilesystemLoop:         return "filesystem loop";
    case ErrorKind::StaleNetworkFileHandle: return "stale network file handle";
    case ErrorKind::InvalidInput:           return "invalid input";
    case ErrorKind::InvalidData:            return "invalid data";
    case ErrorKind::TimedOut:               return "operation timed out";
    case ErrorKind::WriteZero:              return "write zero";
    case ErrorKind::StorageFull:            return "storage full";
    case ErrorKind::NotSeekable:            return "not seekable";
    case ErrorKind::QuotaExceeded:          return "quota exceeded";
    case ErrorKind::FileTooLarge:           return "file too large";
    case ErrorKind::ResourceBusy:           return "resource busy";
    case ErrorKind::ExecutableFileBusy:     return "executable file busy";
    case ErrorKind::Deadlock:               return "deadlock";
    case ErrorKind::CrossesDevices:         return "crosses devices";
    case ErrorKind::TooManyLinks:           return "too many links";
    case ErrorKind::InvalidFilename:        return "invalid filename";
    case ErrorKind::ArgumentListTooLong:    return "argument list too long";
    case ErrorKind::Interrupted:            return "operation interrupted";
    case ErrorKind::Unsupported:            return "operation unsupported";
    case ErrorKind::UnexpectedEof:          return "unexpected end of file";
    case ErrorKind::OutOfMemory:            return "out of memory";
    case ErrorKind::InProgress:             return "operation in progress";
    default:                                return "unknown error";
  }
}

auto Error::from_os_error(int code) -> Error {
  const auto kind = sys_imp::kind_of<ErrorKind>(code);
  return {kind};
}

auto Error::last_os_error() -> Error {
  const auto os_err = sys_imp::get_err();
  return Error::from_os_error(os_err);
}

}  // namespace sfc::io
