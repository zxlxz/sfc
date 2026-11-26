#include <unistd.h>
#include <errno.h>

#include "sfc/sys/posix/io.h"

#define _CVT_R(expr)                           \
  ({                                           \
    auto ret = (expr);                         \
    if (ret == -1) return io::last_os_error(); \
    ret;                                       \
  })

namespace sfc::sys::posix {

auto close(RawFd fd) -> io::Result<> {
  _CVT_R(::close(fd));
  return Ok{};
}

auto read(RawFd fd, Slice<u8> buf) -> io::Result<usize> {
  const auto nret = _CVT_R(::read(fd, buf._ptr, buf._len));
  return usize(nret);
}

auto write(RawFd fd, Slice<const u8> buf) -> io::Result<usize> {
  const auto nret = _CVT_R(::write(fd, buf._ptr, buf._len));
  return usize(nret);
}

auto flush(RawFd fd) -> io::Result<> {
  _CVT_R(::fsync(fd));
  return Ok{};
}

auto seek(RawFd fd, io::SeekFrom pos) -> io::Result<u64> {
  const auto whence = [=]() -> int {
    switch (pos._tag) {
      case io::SeekFrom::Kind::Start:   return SEEK_SET;
      case io::SeekFrom::Kind::Current: return SEEK_CUR;
      case io::SeekFrom::Kind::End:     return SEEK_END;
    }
  }();

  const auto nret = _CVT_R(::lseek(fd, pos.offset, whence));
  return u64(nret);
}

auto is_console(RawFd fd) -> bool {
  const auto ret = ::isatty(fd);
  return ret == 1;
}

auto Stdin() -> File {
  return File{STDIN_FILENO};
}

auto Stdout() -> File {
  return File{STDOUT_FILENO};
}

auto Stderr() -> File {
  return File{STDERR_FILENO};
}

auto os_error() -> int {
  return errno;
}

auto io_error(int code) -> io::Error {
  switch (code) {
    case 0:             return io::Error::Success;
    case ENOENT:        return io::Error::NotFound;
    case EACCES:
    case EPERM:         return io::Error::PermissionDenied;
    case ECONNREFUSED:  return io::Error::ConnectionRefused;
    case ECONNRESET:    return io::Error::ConnectionReset;
    case ECONNABORTED:  return io::Error::ConnectionAborted;
    case ENOTCONN:      return io::Error::NotConnected;
    case EADDRINUSE:    return io::Error::AddrInUse;
    case EADDRNOTAVAIL: return io::Error::AddrNotAvailable;
    case ENETUNREACH:   return io::Error::NetworkUnreachable;
    case EHOSTUNREACH:  return io::Error::HostUnreachable;
    case ENETDOWN:      return io::Error::NetworkDown;
    case EPIPE:         return io::Error::BrokenPipe;
    case EEXIST:        return io::Error::AlreadyExists;
    case EWOULDBLOCK:   return io::Error::WouldBlock;
    case EINVAL:        return io::Error::InvalidInput;
    case EBADMSG:       return io::Error::InvalidData;
    case ENOTSUP:       return io::Error::InvalidOperation;
    case EINTR:         return io::Error::Interrupted;
    case ENOSYS:        return io::Error::Unsupported;
    case ENOSPC:        return io::Error::WriteZero;
    case ETIMEDOUT:     return io::Error::TimedOut;
    case EISDIR:        return io::Error::IsADirectory;
    case ENOTDIR:       return io::Error::NotADirectory;
    case ENOTEMPTY:     return io::Error::DirectoryNotEmpty;
    case ESPIPE:        return io::Error::NotSeekable;
    case EFBIG:         return io::Error::FileTooLarge;
    case EBUSY:         return io::Error::ResourceBusy;
    case EDEADLK:       return io::Error::Deadlock;
    case ENOMEM:        return io::Error::OutOfMemory;
    case EINPROGRESS:   return io::Error::InProgress;
    default:            return io::Error::Other;
  }
}

}  // namespace sfc::sys::posix
