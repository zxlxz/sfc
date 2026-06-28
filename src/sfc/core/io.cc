#include "sfc/core/io.h"
#include "sfc/core/str.h"
#include "sfc/core/result.h"

namespace sfc::io {

auto to_str(Error e) noexcept -> Str {
  switch (e) {
    case Error::Success:            return "Success";
    case Error::NotFound:           return "NotFound";
    case Error::PermissionDenied:   return "PermissionDenied";
    case Error::ConnectionRefused:  return "ConnectionRefused";
    case Error::ConnectionReset:    return "ConnectionReset";
    case Error::ConnectionAborted:  return "ConnectionAborted";
    case Error::NotConnected:       return "NotConnected";
    case Error::AddrInUse:          return "AddrInUse";
    case Error::AddrNotAvailable:   return "AddrNotAvailable";
    case Error::NetworkUnreachable: return "NetworkUnreachable";
    case Error::HostUnreachable:    return "HostUnreachable";
    case Error::NetworkDown:        return "NetworkDown";
    case Error::BrokenPipe:         return "BrokenPipe";
    case Error::AlreadyExists:      return "AlreadyExists";
    case Error::WouldBlock:         return "WouldBlock";
    case Error::InvalidInput:       return "InvalidInput";
    case Error::InvalidData:        return "InvalidData";
    case Error::InvalidOperation:   return "InvalidOperation";
    case Error::Interrupted:        return "Interrupted";
    case Error::Unsupported:        return "Unsupported";
    case Error::UnexpectedEof:      return "UnexpectedEof";
    case Error::WriteZero:          return "WriteZero";
    case Error::TimedOut:           return "TimedOut";
    case Error::IsADirectory:       return "IsADirectory";
    case Error::NotADirectory:      return "NotADirectory";
    case Error::DirectoryNotEmpty:  return "DirectoryNotEmpty";
    case Error::NotSeekable:        return "NotSeekable";
    case Error::FileTooLarge:       return "FileTooLarge";
    case Error::ResourceBusy:       return "ResourceBusy";
    case Error::Deadlock:           return "Deadlock";
    case Error::StorageFull:        return "StorageFull";
    case Error::OutOfMemory:        return "OutOfMemory";
    case Error::InProgress:         return "InProgress";
    case Error::Other:              return "Other";
    default:                 return "Unknown";
  }
}

}  // namespace sfc::io

namespace sfc::slice {

template <class T>
auto Slice<T>::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (_len == 0 || buf._len == 0) {
    return io::Result<usize>{0UZ};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return io::Result<usize>{amt};
}

template <class T>
auto Slice<const T>::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  if (_len == 0 || buf._len == 0) {
    return io::Result<usize>{0UZ};
  }
  const auto amt = _len < buf._len ? _len : buf._len;
  ptr::copy_nonoverlapping(_ptr, buf._ptr, amt);
  _ptr += amt;
  _len -= amt;
  return io::Result<usize>{amt};
}

template auto Slice<u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;
template auto Slice<const u8>::read(Slice<u8> buf) noexcept -> io::Result<usize>;

}  // namespace sfc::slice
