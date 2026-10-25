#include <unistd.h>
#include <errno.h>

#include "sfc/sys/posix/io.h"

namespace sfc::sys::posix {

void File::close() {
  if (_fd == -1) {
    return;
  }

  (void)::close(_fd);
  _fd = -1;
}

auto File::read(Slice<u8> buf) -> io::Result<usize> {
  const auto nret = ::read(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return io::last_os_error();
  }
  return usize(nret);
}

auto File::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto nret = ::write(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return io::last_os_error();
  }
  return usize(nret);
}

auto File::flush() -> io::Result<> {
  const auto ret = ::fsync(_fd);
  if (ret == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto File::seek(io::SeekFrom pos) -> io::Result<u64> {
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
  return u64(nret);
}

auto StdIo::Input() noexcept -> StdIo {
  const auto fd = STDIN_FILENO;
  return StdIo{._file = {fd}};
}

auto StdIo::Output() noexcept -> StdIo {
  const auto fd = STDOUT_FILENO;
  return StdIo{._file = {fd}};
}

auto StdIo::Error() noexcept -> StdIo {
  const auto fd = STDERR_FILENO;
  return StdIo{._file = {fd}};
}

auto StdIo::is_console() -> bool {
  const auto ret = ::isatty(_file._fd);
  return ret == 1;
}

auto StdIo::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return _file.read(buf);
}

auto StdIo::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return _file.write(buf);
}

auto StdIo::flush() noexcept -> io::Result<> {
  return _file.flush();
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
