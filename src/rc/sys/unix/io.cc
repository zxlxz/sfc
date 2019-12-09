#include "rc/sys/unix.inl"

#include "rc/io.h"

namespace rc::sys::unix::io {

auto get_err_kind(i32 code) -> ErrorKind {
  switch (code) {
    case ECONNREFUSED:
      return ErrorKind::ConnectionRefused;
    case ECONNRESET:
      return ErrorKind::ConnectionReset;
    case EPERM:
      return ErrorKind::PermissionDenied;
    case EACCES:
      return ErrorKind::PermissionDenied;
    case EPIPE:
      return ErrorKind::BrokenPipe;
    case ENOTCONN:
      return ErrorKind::NotConnected;
    case ECONNABORTED:
      return ErrorKind::ConnectionAborted;
    case EADDRNOTAVAIL:
      return ErrorKind::AddrNotAvailable;
    case EADDRINUSE:
      return ErrorKind::AddrInUse;
    case ENOENT:
      return ErrorKind::NotFound;
    case EINTR:
      return ErrorKind::Interrupted;
    case EINVAL:
      return ErrorKind::InvalidInput;
    case ETIMEDOUT:
      return ErrorKind::TimedOut;
    case EEXIST:
      return ErrorKind::AlreadyExists;
    default:
      break;
  };

  if (code == EAGAIN || code == EWOULDBLOCK) {
    return ErrorKind::WouldBlock;
  }

  return ErrorKind::Other;
}

auto Stdin::read(Slice<u8> buf) -> usize {
  static auto f = fs::File{FileDesc(STDIN_FILENO)};
  const auto res = f.read(buf);
  return res;
}

auto Stdout::write(Slice<const u8> buf) -> usize {
  static auto f = fs::File{FileDesc(STDOUT_FILENO)};
  const auto res = f.write(buf);
  return res;
}

auto Stderr::write(Slice<const u8> buf) -> usize {
  static auto f = fs::File{FileDesc(STDERR_FILENO)};
  const auto res = f.write(buf);
  return res;
}

auto get_console_columns() -> u32 {
  struct ::winsize console_info {};
  ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &console_info);
  return u32(console_info.ws_col);
}

}  // namespace rc::sys::unix::io
