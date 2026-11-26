#include <Windows.h>

#include "sfc/sys/windows/io.h"

#define _CVT_R(expr)                              \
  ({                                              \
    auto ret = (expr);                            \
    if (ret == FALSE) return io::last_os_error(); \
    ret;                                          \
  })

namespace sfc::sys::windows {

auto close(RawFd fd) -> io::Result<> {
  _CVT_R(::CloseHandle(fd));
  return Ok{};
}

auto read(RawFd fd, Slice<u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nread = 0UL;
  _CVT_R(::ReadFile(fd, buf_ptr, buf_len, &nread, nullptr));
  return usize{nread};
}

auto write(RawFd fd, Slice<const u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nwrite = 0UL;
  _CVT_R(::WriteFile(fd, buf_ptr, buf_len, &nwrite, nullptr));
  return usize{nwrite};
}

auto flush(RawFd fd) -> io::Result<> {
  _CVT_R(::FlushFileBuffers(fd));
  return Ok{};
}

auto seek(RawFd fd, io::SeekFrom pos) -> io::Result<u64> {
  const auto move_method = [=] -> DWORD {
    switch (pos._tag) {
      case io::SeekFrom::Kind::Start:   return FILE_BEGIN;
      case io::SeekFrom::Kind::Current: return FILE_CURRENT;
      case io::SeekFrom::Kind::End:     return FILE_END;
    }
  }();

  const auto old_pos = LARGE_INTEGER{.QuadPart = pos.offset};
  auto new_pos = LARGE_INTEGER{};
  _CVT_R(::SetFilePointerEx(fd, old_pos, &new_pos, move_method));

  const auto ret_pos = num::cast_unsigned(new_pos.QuadPart);
  return u64{ret_pos};
}

auto is_console(RawFd fd) -> bool {
  auto mode = DWORD{};
  return ::GetConsoleMode(fd, &mode);
}

auto std_file(DWORD id) -> File {
  const auto handle = ::GetStdHandle(id);
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return File{kInvalidFd};
  }
  return File{handle};
}

auto Stdin() -> File {
  return std_file(STD_INPUT_HANDLE);
}

auto Stdout() -> File {
  return std_file(STD_OUTPUT_HANDLE);
}

auto Stderr() -> File {
  return std_file(STD_ERROR_HANDLE);
}

auto os_error() -> u32 {
  const auto err = ::GetLastError();
  return u32(err);
}

auto io_error(u32 code) -> io::Error {
  switch (code) {
    case 0:                         return io::Error::Success;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
    case ERROR_BAD_NETPATH:
    case ERROR_BAD_NET_NAME:
    case ERROR_BAD_PATHNAME:        return io::Error::NotFound;
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case WSAEACCES:                 return io::Error::PermissionDenied;
    case WSAECONNREFUSED:
    case ERROR_CONNECTION_REFUSED:  return io::Error::ConnectionRefused;
    case WSAECONNRESET:
    case ERROR_NETNAME_DELETED:     return io::Error::ConnectionReset;
    case WSAECONNABORTED:
    case ERROR_CONNECTION_ABORTED:  return io::Error::ConnectionAborted;
    case WSAENOTCONN:
    case ERROR_NOT_CONNECTED:       return io::Error::NotConnected;
    case WSAEADDRINUSE:
    case ERROR_DUP_NAME:            return io::Error::AddrInUse;
    case WSAEADDRNOTAVAIL:          return io::Error::AddrNotAvailable;
    case WSAENETUNREACH:
    case ERROR_NETWORK_UNREACHABLE: return io::Error::NetworkUnreachable;
    case WSAEHOSTUNREACH:
    case ERROR_HOST_UNREACHABLE:    return io::Error::HostUnreachable;
    case WSAENETDOWN:
    case ERROR_NETWORK_BUSY:        return io::Error::NetworkDown;
    case ERROR_BROKEN_PIPE:
    case ERROR_NO_DATA:             return io::Error::BrokenPipe;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:      return io::Error::AlreadyExists;
    case WSAEWOULDBLOCK:            return io::Error::WouldBlock;
    case ERROR_INVALID_PARAMETER:
    case WSAEINVAL:                 return io::Error::InvalidInput;
    case ERROR_INVALID_FUNCTION:    return io::Error::InvalidOperation;
    case ERROR_OPERATION_ABORTED:
    case WSAEINTR:                  return io::Error::Interrupted;
    case ERROR_NOT_SUPPORTED:       return io::Error::Unsupported;
    case ERROR_NO_MORE_FILES:
    case ERROR_HANDLE_EOF:          return io::Error::UnexpectedEof;
    case WSAETIMEDOUT:
    case ERROR_SEM_TIMEOUT:         return io::Error::TimedOut;
    case ERROR_FILE_TOO_LARGE:      return io::Error::FileTooLarge;
    case ERROR_DIRECTORY:           return io::Error::NotADirectory;
    case ERROR_DIR_NOT_EMPTY:       return io::Error::DirectoryNotEmpty;
    case ERROR_DEVICE_IN_USE:       return io::Error::ResourceBusy;
    case ERROR_POSSIBLE_DEADLOCK:   return io::Error::Deadlock;
    case ERROR_DISK_FULL:
    case ERROR_HANDLE_DISK_FULL:    return io::Error::StorageFull;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
    case WSAENOBUFS:                return io::Error::OutOfMemory;
    case ERROR_IO_INCOMPLETE:       return io::Error::InProgress;
    default:                        return io::Error::Other;
  }
}

}  // namespace sfc::sys::windows
