#include <unistd.h>
#include <errno.h>

#include "sfc/sys/posix/io.h"

namespace sfc::sys::posix {

File::File(int fd) noexcept : _fd(fd) {}

File::~File() noexcept {
  if (_fd == -1) return;
  ::close(_fd);
}

File::File(File&& other) noexcept : _fd(other._fd) {
  other._fd = -1;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    mem::swap(_fd, other._fd);
  }
  return *this;
}

auto File::is_valid() const noexcept -> bool {
  return _fd != -1;
}

auto File::flush() noexcept -> io::Result<> {
  if (::fsync(_fd) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  const auto nret = ::read(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return Err{io::last_os_error()};
  }
  const auto nread = num::cast_unsigned(nret);
  return Ok{nread};
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  const auto nret = ::write(_fd, buf._ptr, buf._len);
  if (nret == -1) {
    return {io::last_os_error()};
  }
  const auto nwrite = num::cast_unsigned(nret);
  return {nwrite};
}

auto File::seek(i64 offset, int whence) noexcept -> io::Result<usize> {
  const auto ret = ::lseek(_fd, offset, whence);
  if (ret == -1) {
    return io::last_os_error();
  }
  const auto nseek = num::cast_unsigned(ret);
  return {usize{nseek}};
}

auto StdIn::is_console() -> bool {
  return ::isatty(STDIN_FILENO) == 1;
}

auto StdIn::read(Slice<u8> data) -> io::Result<usize> {
  const auto nret = ::read(STDIN_FILENO, data._ptr, data._len);
  if (nret == -1) {
    return io::last_os_error();
  }

  const auto nread = num::cast_unsigned(nret);
  return usize{nread};
}

auto Stdout::is_console() -> bool {
  return ::isatty(STDOUT_FILENO) == 1;
}

auto Stdout::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto ret = ::write(STDOUT_FILENO, buf._ptr, buf._len);
  if (ret == -1) {
    return io::last_os_error();
  }

  const auto nwrite = num::cast_unsigned(ret);
  return usize{nwrite};
}

auto Stderr::is_console() -> bool {
  return ::isatty(STDERR_FILENO) == 1;
}

auto Stderr::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto ret = ::write(STDERR_FILENO, buf._ptr, buf._len);
  if (ret == -1) {
    return io::last_os_error();
  }

  const auto nwrite = num::cast_unsigned(ret);
  return usize{nwrite};
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
