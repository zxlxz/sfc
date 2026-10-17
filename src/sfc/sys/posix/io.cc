#include <unistd.h>
#include <errno.h>

#include "sfc/sys/posix/io.h"

namespace sfc::sys::posix {

auto File::close() noexcept -> io::Result<> {
  const auto ret = ::close(_fd);
  if (ret == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  const auto nret = ::read(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return io::last_os_error();
  }
  const auto nread = num::cast_unsigned(nret);
  return Ok{nread};
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  const auto nret = ::write(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return io::last_os_error();
  }
  return Ok{usize(nret)};
}

auto File::flush() noexcept -> io::Result<> {
  const auto ret = ::fsync(_fd);
  if (ret == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<usize> {
  const auto whence = [=]() {
    switch (pos._tag) {
      case io::SeekFrom::Kind::Start:   return SEEK_SET;
      case io::SeekFrom::Kind::Current: return SEEK_CUR;
      case io::SeekFrom::Kind::End:     return SEEK_END;
    }
    return SEEK_SET;
  }();

  const auto nret = ::lseek(_fd, pos.offset, whence);
  if (nret == -1) {
    return io::last_os_error();
  }
  return Ok{usize(nret)};
}

auto StdIo::is_console() -> bool {
  const auto ret = ::isatty(_fd);
  return ret == 1;
}

auto StdIo::Input() noexcept -> StdIo {
  return StdIo{{._fd = 0}};
}

auto StdIo::Output() noexcept -> StdIo {
  return StdIo{{._fd = 1}};
}

auto StdIo::Error() noexcept -> StdIo {
  return StdIo{{._fd = 2}};
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
