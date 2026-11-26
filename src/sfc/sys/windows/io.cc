#include <Windows.h>

#include "sfc/sys/windows/io.h"

#define _CVT_R(expr) \
  if (!(expr)) return io_error(os_error());

namespace sfc::sys::windows {

void File::close() {
  if (_fd == nullptr || _fd == INVALID_HANDLE_VALUE) {
    return;
  }
  ::CloseHandle(_fd);
  _fd = nullptr;
}

auto File::read(Slice<u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nread = 0UL;
  _CVT_R(::ReadFile(_fd, buf_ptr, buf_len, &nread, nullptr));
  return usize{nread};
}

auto File::write(Slice<const u8> buf) -> io::Result<usize> {
  const auto buf_ptr = buf._ptr;
  const auto buf_len = u32(cmp::min(buf._len, U32::MAX));

  auto nwrite = 0UL;
  _CVT_R(::WriteFile(_fd, buf_ptr, buf_len, &nwrite, nullptr));
  return usize{nwrite};
}

auto File::flush() -> io::Result<> {
  _CVT_R(::FlushFileBuffers(_fd));
  return Ok{};
}

auto File::seek(io::SeekFrom pos) -> io::Result<u64> {
  const auto move_method = [=] -> DWORD {
    switch (pos._tag) {
      case io::SeekFrom::Kind::Start:   return FILE_BEGIN;
      case io::SeekFrom::Kind::Current: return FILE_CURRENT;
      case io::SeekFrom::Kind::End:     return FILE_END;
    }
  }();

  const auto old_pos = LARGE_INTEGER{.QuadPart = pos.offset};
  auto new_pos = LARGE_INTEGER{};
  _CVT_R(::SetFilePointerEx(_fd, old_pos, &new_pos, move_method));

  const auto ret_pos = num::cast_unsigned(new_pos.QuadPart);
  return u64{ret_pos};
}

auto File::is_console() const -> bool {
  auto mode = DWORD{};
  return ::GetConsoleMode(_fd, &mode);
}

auto Stdin() -> File& {
  static auto res = File{._fd = ::GetStdHandle(STD_INPUT_HANDLE)};
  return res;
}

auto Stdout() -> File& {
  static auto res = File{._fd = ::GetStdHandle(STD_OUTPUT_HANDLE)};
  return res;
}

auto Stderr() -> File& {
  static auto res = File{._fd = ::GetStdHandle(STD_ERROR_HANDLE)};
  return res;
}

auto os_error() -> u32 {
  auto err = ::GetLastError();
  return u32(err);
}

auto io_error(u32 code) -> io::Error {
  switch (code) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:    return io::Error::NotFound;
    case ERROR_ACCESS_DENIED:     return io::Error::PermissionDenied;
    case WSAECONNREFUSED:         return io::Error::ConnectionRefused;
    case WSAECONNRESET:           return io::Error::ConnectionReset;
    case WSAECONNABORTED:         return io::Error::ConnectionAborted;
    case WSAENOTCONN:             return io::Error::NotConnected;
    case WSAEADDRINUSE:           return io::Error::AddrInUse;
    case WSAEADDRNOTAVAIL:        return io::Error::AddrNotAvailable;
    case WSAENETUNREACH:          return io::Error::NetworkUnreachable;
    case WSAEHOSTUNREACH:         return io::Error::HostUnreachable;
    case WSAENETDOWN:             return io::Error::NetworkDown;
    case ERROR_IO_DEVICE:         return io::Error::BrokenPipe;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:    return io::Error::AlreadyExists;
    case WSAEWOULDBLOCK:          return io::Error::WouldBlock;
    case ERROR_INVALID_PARAMETER: return io::Error::InvalidInput;
    case ERROR_INVALID_FUNCTION:  return io::Error::InvalidOperation;
    case ERROR_OPERATION_ABORTED: return io::Error::Interrupted;
    case ERROR_NOT_SUPPORTED:     return io::Error::Unsupported;
    case ERROR_NO_MORE_FILES:
    case ERROR_HANDLE_EOF:        return io::Error::UnexpectedEof;
    case ERROR_DISK_FULL:         return io::Error::StorageFull;
    case ERROR_DIRECTORY:         return io::Error::NotADirectory;
    case ERROR_DIR_NOT_EMPTY:     return io::Error::DirectoryNotEmpty;
    case ERROR_DEVICE_IN_USE:     return io::Error::ResourceBusy;
    case ERROR_POSSIBLE_DEADLOCK: return io::Error::Deadlock;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:       return io::Error::StorageFull;
    case ERROR_IO_INCOMPLETE:     return io::Error::InProgress;
    case WSAETIMEDOUT:            return io::Error::TimedOut;
    default:                      return io::Error::Other;
  }
}

}  // namespace sfc::sys::windows
