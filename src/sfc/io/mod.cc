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
    case NotADirectory:          return "NotADirectory";
    case IsADirectory:           return "IsADirectory";
    case DirectoryNotEmpty:      return "DirectoryNotEmpty";
    case ReadOnlyFilesystem:     return "ReadOnlyFilesystem";
    case FilesystemLoop:         return "FilesystemLoop";
    case StaleNetworkFileHandle: return "StaleNetworkFileHandle";
    case InvalidInput:           return "InvalidInput";
    case InvalidData:            return "InvalidData";
    case TimedOut:               return "TimedOut";
    case WriteZero:              return "WriteZero";
    case StorageFull:            return "StorageFull";
    case NotSeekable:            return "NotSeekable";
    case QuotaExceeded:          return "QuotaExceeded";
    case FileTooLarge:           return "FileTooLarge";
    case ResourceBusy:           return "ResourceBusy";
    case ExecutableFileBusy:     return "ExecutableFileBusy";
    case Deadlock:               return "Deadlock";
    case CrossesDevices:         return "CrossesDevices";
    case TooManyLinks:           return "TooManyLinks";
    case InvalidFilename:        return "InvalidFilename";
    case ArgumentListTooLong:    return "ArgumentListTooLong";
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
