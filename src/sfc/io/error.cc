#include "error.h"

#include <errno.h>

namespace sfc::io {

Error::Error(int id) : _id{id} {}

auto Error::last_os_error() -> Error {
  const auto eid = errno;
  return Error{eid};
}

auto Error::to_str() const -> Str {
  switch (_id) {
      // clang-format off
    case EPERM:         return "io::Error::PermissionDenied";
    case ENOENT:        return "io::Error::NotFound";
    case EINTR:         return "io::Error::Interrupted";
    case EWOULDBLOCK:   return "io::Error::WouldBlock";
    case EACCES:        return "io::Error::PermissionDenied";
    case EEXIST:        return "io::Error::AlreadyExists";
    case EINVAL:        return "io::Error::InvalidInput";
    case EPIPE:         return "io::Error::BrokenPipe";
    case EADDRNOTAVAIL: return "io::Error::AddrNotAvailable";
    case ENOTCONN:      return "io::Error::NotConnected";
    case ECONNABORTED:  return "io::Error::ConnectionAborted";
    case ECONNRESET:    return "io::Error::ConnectionReset";
    case EADDRINUSE:    return "io::Error::AddrInUse";
    case ETIMEDOUT:     return "io::Error::TimedOut";
    case ECONNREFUSED:  return "io::Error::ConnectionRefused";
    default:            return "io::Error::Unknown";
      // clang-format on
  }
}

}  // namespace sfc::io
