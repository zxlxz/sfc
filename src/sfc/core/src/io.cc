#include "sfc/core/io.h"
#include "sfc/core/str.h"

namespace sfc::io {

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

namespace sfc::slice {

template <class T>
auto Slice<T>::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (_len == 0 || buf._len == 0) {
    return Ok{0UZ};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return Ok{amt};
}

template auto Slice<u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
template auto Slice<const u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;

}  // namespace sfc::slice
