#include "error.h"

#include "sfc/sys/io.inl"

namespace sfc::io {

namespace sys_imp = sys::io;

SFC_ENUM(ErrorKind,  //
         NotFound, PermissionDenied, ConnectionRefused, ConnectionReset, HostUnreachable,
         NetworkUnreachable, ConnectionAborted, NotConnected, AddrInUse, AddrNotAvailable,
         NetworkDown, BrokenPipe, AlreadyExists, WouldBlock, NotADirectory, IsADirectory,
         DirectoryNotEmpty, ReadOnlyFilesystem, FilesystemLoop, StaleNetworkFileHandle,
         InvalidInput, InvalidData, TimedOut, WriteZero, StorageFull, NotSeekable,
         FilesystemQuotaExceeded, FileTooLarge, ResourceBusy, ExecutableFileBusy, Deadlock,
         CrossesDevices, TooManyLinks, InvalidFilename, ArgumentListTooLong, Interrupted,
         Unsupported, UnexpectedEof, OutOfMemory, Other);

auto Error::last_os_error() -> Error {
  const auto code = sys_imp::errno();
  return Error::from_os_error(code);
}

auto Error::from_os_error(int code) -> Error {
  return {ErrorKind::Other, code};
}

auto Error::kind() const -> ErrorKind {
  return _kind;
}

auto Error::as_str() const -> Str {
  if (_code != 0) {
    return sys_imp::error_str(_code);
  }
  return reflect::enum_name(_kind);
}

}  // namespace sfc::io
