#include "rc/sys/windows.inl"

#include "rc/io.h"

namespace rc::sys::windows::io {

using rc::io::ErrorKind;

auto get_err_kind(i32 code) -> ErrorKind {
  switch (code) {
    case ERROR_ACCESS_DENIED:
      return ErrorKind::PermissionDenied;
    case ERROR_ALREADY_EXISTS:
      return ErrorKind::AlreadyExists;
    case ERROR_FILE_EXISTS:
      return ErrorKind::AlreadyExists;
    case ERROR_BROKEN_PIPE:
      return ErrorKind::BrokenPipe;
    case ERROR_FILE_NOT_FOUND:
      return ErrorKind::NotFound;
    case ERROR_PATH_NOT_FOUND:
      return ErrorKind::NotFound;
    case ERROR_NO_DATA:
      return ErrorKind::BrokenPipe;
    case ERROR_OPERATION_ABORTED:
      return ErrorKind::TimedOut;
  }

  switch (code) {
    case WSAEACCES:
      return ErrorKind::PermissionDenied;
    case WSAEADDRINUSE:
      return ErrorKind::AddrInUse;
    case WSAEADDRNOTAVAIL:
      return ErrorKind::AddrNotAvailable;
    case WSAECONNABORTED:
      return ErrorKind::ConnectionAborted;
    case WSAECONNREFUSED:
      return ErrorKind::ConnectionRefused;
    case WSAECONNRESET:
      return ErrorKind::ConnectionReset;
    case WSAEINVAL:
      return ErrorKind::InvalidInput;
    case WSAENOTCONN:
      return ErrorKind::NotConnected;
    case WSAEWOULDBLOCK:
      return ErrorKind::WouldBlock;
    case WSAETIMEDOUT:
      return ErrorKind::TimedOut;
  }
  return ErrorKind::Other;
}

static auto get_handle(DWORD handle_id) -> HANDLE {
  auto handle = ::GetStdHandle(handle_id);

  if (handle == nullptr) {
    return handle;
  }

  if (handle_id == STD_OUTPUT_HANDLE || handle_id == STD_ERROR_HANDLE) {
    auto mode = DWORD(0);
    ::GetConsoleMode(handle, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    ::SetConsoleMode(handle, mode);
  }

  return handle;
}

auto Stdin::read(Slice<u8> buf) -> usize {
  static auto handle = sys::io::get_handle(STD_INPUT_HANDLE);

  if (buf.is_empty()) {
    return 0;
  }

  const auto buf_ptr = buf.as_mut_ptr();
  const auto buf_len = DWORD(buf.len());
  auto amount = DWORD(0);
  const auto r = ::ReadConsoleA(handle, buf_ptr, buf_len, &amount, nullptr);
  if (r == FALSE) {
    return 0;
  }
  return usize(amount);
}

auto Stdout::write(Slice<const u8> buf) -> usize {
  static auto handle = sys::io::get_handle(STD_OUTPUT_HANDLE);

  if (buf.is_empty()) {
    return 0;
  }

  const auto buf_ptr = buf.as_ptr();
  const auto buf_len = DWORD(buf.len());
  auto amount = DWORD(0u);
  const auto r = ::WriteConsoleA(handle, buf_ptr, buf_len, &amount, nullptr);
  if (r == FALSE) {
    return 0;
  }
  return usize(amount);
}

auto Stderr::write(Slice<const u8> buf) -> usize {
  static auto handle = sys::io::get_handle(STD_ERROR_HANDLE);

  if (buf.is_empty()) {
    return 0;
  }

  const auto buf_ptr = buf.as_ptr();
  const auto buf_len = DWORD(buf.len());
  auto amount = DWORD(0u);
  const auto r = ::WriteConsoleA(handle, buf_ptr, buf_len, &amount, nullptr);
  if (r == FALSE) {
    return 0;
  }
  return usize(amount);
}

}  // namespace rc::sys::windows::io
