#include "sfc/core/io.h"
#include "sfc/core/str.h"
#include "sfc/core/result.h"

namespace sfc::io {

auto to_str(Error e) noexcept -> Str {
  switch (e) {
    case Error::Success:            return "io::Error::Success";
    case Error::NotFound:           return "io::Error::NotFound";
    case Error::PermissionDenied:   return "io::Error::PermissionDenied";
    case Error::ConnectionRefused:  return "io::Error::ConnectionRefused";
    case Error::ConnectionReset:    return "io::Error::ConnectionReset";
    case Error::ConnectionAborted:  return "io::Error::ConnectionAborted";
    case Error::NotConnected:       return "io::Error::NotConnected";
    case Error::AddrInUse:          return "io::Error::AddrInUse";
    case Error::AddrNotAvailable:   return "io::Error::AddrNotAvailable";
    case Error::NetworkUnreachable: return "io::Error::NetworkUnreachable";
    case Error::HostUnreachable:    return "io::Error::HostUnreachable";
    case Error::NetworkDown:        return "io::Error::NetworkDown";
    case Error::BrokenPipe:         return "io::Error::BrokenPipe";
    case Error::AlreadyExists:      return "io::Error::AlreadyExists";
    case Error::WouldBlock:         return "io::Error::WouldBlock";
    case Error::InvalidInput:       return "io::Error::InvalidInput";
    case Error::InvalidData:        return "io::Error::InvalidData";
    case Error::InvalidOperation:   return "io::Error::InvalidOperation";
    case Error::Interrupted:        return "io::Error::Interrupted";
    case Error::Unsupported:        return "io::Error::Unsupported";
    case Error::UnexpectedEof:      return "io::Error::UnexpectedEof";
    case Error::WriteZero:          return "io::Error::WriteZero";
    case Error::TimedOut:           return "io::Error::TimedOut";
    case Error::IsADirectory:       return "io::Error::IsADirectory";
    case Error::NotADirectory:      return "io::Error::NotADirectory";
    case Error::DirectoryNotEmpty:  return "io::Error::DirectoryNotEmpty";
    case Error::NotSeekable:        return "io::Error::NotSeekable";
    case Error::FileTooLarge:       return "io::Error::FileTooLarge";
    case Error::ResourceBusy:       return "io::Error::ResourceBusy";
    case Error::Deadlock:           return "io::Error::Deadlock";
    case Error::StorageFull:        return "io::Error::StorageFull";
    case Error::OutOfMemory:        return "io::Error::OutOfMemory";
    case Error::InProgress:         return "io::Error::InProgress";
    case Error::Other:              return "io::Error::Other";
    default:                        return "io::Error::Unknown";
  }
}

}  // namespace sfc::io
