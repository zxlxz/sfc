#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

enum class ErrorKind : i32 {
  NotFound,
  PermissionDenied,
  ConnectionRefused,
  ConnectionReset,
  HostUnreachable,
  NetworkUnreachable,
  ConnectionAborted,
  NotConnected,
  AddrInUse,
  AddrNotAvailable,
  NetworkDown,
  BrokenPipe,
  AlreadyExists,
  WouldBlock,
  NotADirectory,
  IsADirectory,
  DirectoryNotEmpty,
  ReadOnlyFilesystem,
  FilesystemLoop,
  StaleNetworkFileHandle,
  InvalidInput,
  InvalidData,
  TimedOut,
  WriteZero,
  StorageFull,
  NotSeekable,
  FilesystemQuotaExceeded,
  FileTooLarge,
  ResourceBusy,
  ExecutableFileBusy,
  Deadlock,
  CrossesDevices,
  TooManyLinks,
  InvalidFilename,
  ArgumentListTooLong,
  Interrupted,
  Unsupported,
  UnexpectedEof,
  OutOfMemory,
  Other,
};

struct Error {
  ErrorKind _kind;
  int _code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  auto kind() const -> ErrorKind;

  auto as_str() const -> Str;

  void fmt(auto& f) const {
    const auto s = this->as_str();
    f.write(s);
  }
};

}  // namespace sfc::io
