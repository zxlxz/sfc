#include "sfc/core.h"
#include "sfc/io/mod.h"
#include "sfc/sys/io.h"

namespace sfc::io {

namespace sys_imp = sys::io;

auto to_str(Error val) noexcept -> Str {
  using enum Error;
  switch (val) {
    case Success:                return "Success";
    case NotFound:               return "NotFound";
    case PermissionDenied:       return "PermissionDenied";
    case ConnectionRefused:      return "ConnectionRefused";
    case ConnectionReset:        return "ConnectionReset";
    case HostUnreachable:        return "HostUnreachable";
    case NetworkUnreachable:     return "NetworkUnreachable";
    case ConnectionAborted:      return "ConnectionAborted";
    case NotConnected:           return "NotConnected";
    case AddrInUse:              return "AddrInUse";
    case AddrNotAvailable:       return "AddrNotAvailable";
    case NetworkDown:            return "NetworkDown";
    case BrokenPipe:             return "BrokenPipe";
    case AlreadyExists:          return "AlreadyExists";
    case WouldBlock:             return "WouldBlock";
    case DirectoryNotEmpty:      return "DirectoryNotEmpty";
    case InvalidInput:           return "InvalidInput";
    case InvalidData:            return "InvalidData";
    case TimedOut:               return "TimedOut";
    case WriteZero:              return "WriteZero";
    case StorageFull:            return "StorageFull";
    case NotSeekable:            return "NotSeekable";
    case FileTooLarge:           return "FileTooLarge";
    case ResourceBusy:           return "ResourceBusy";
    case Deadlock:               return "Deadlock";
    case Interrupted:            return "Interrupted";
    case Unsupported:            return "Unsupported";
    case UnexpectedEof:          return "UnexpectedEof";
    case OutOfMemory:            return "OutOfMemory";
    case InProgress:             return "InProgress";
    case Other:                  return "Other";
    default:                     return "Unknown";
  }
}

auto last_os_error() noexcept -> Error {
  const auto os_err = sys_imp::get_err();
  const auto io_err = sys_imp::map_err<Error>(os_err);
  return io_err;
}

}  // namespace sfc::io
