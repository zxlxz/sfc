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

auto Error::kind() const noexcept -> ErrorKind {
  return _kind;
}

auto Error::raw_os_err() const noexcept -> ErrCode {
  return _code;
}

auto Error::to_str() const noexcept -> Str {
  using enum ErrorKind;
  switch (_kind) {
    case Success:            return "Success";
    case NotFound:           return "NotFound";
    case PermissionDenied:   return "PermissionDenied";
    case ConnectionRefused:  return "ConnectionRefused";
    case ConnectionReset:    return "ConnectionReset";
    case ConnectionAborted:  return "ConnectionAborted";
    case NotConnected:       return "NotConnected";
    case AddrInUse:          return "AddrInUse";
    case AddrNotAvailable:   return "AddrNotAvailable";
    case NetworkUnreachable: return "NetworkUnreachable";
    case HostUnreachable:    return "HostUnreachable";
    case NetworkDown:        return "NetworkDown";
    case BrokenPipe:         return "BrokenPipe";
    case AlreadyExists:      return "AlreadyExists";
    case WouldBlock:         return "WouldBlock";
    case InvalidInput:       return "InvalidInput";
    case InvalidData:        return "InvalidData";
    case InvalidOperation:   return "InvalidOperation";
    case Interrupted:        return "Interrupted";
    case Unsupported:        return "Unsupported";
    case UnexpectedEof:      return "UnexpectedEof";
    case WriteZero:          return "WriteZero";
    case TimedOut:           return "TimedOut";
    case IsADirectory:       return "IsADirectory";
    case NotADirectory:      return "NotADirectory";
    case DirectoryNotEmpty:  return "DirectoryNotEmpty";
    case NotSeekable:        return "NotSeekable";
    case FileTooLarge:       return "FileTooLarge";
    case ResourceBusy:       return "ResourceBusy";
    case Deadlock:           return "Deadlock";
    case StorageFull:        return "StorageFull";
    case OutOfMemory:        return "OutOfMemory";
    case InProgress:         return "InProgress";
    case Other:              return "Other";
    default:                 return "Unknown";
  }
}

}  // namespace sfc::io
