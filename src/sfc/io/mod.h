
#pragma once

#include "sfc/core.h"

namespace sfc::io {

enum class ErrorKind : i32 {
  Other = -1,
  NotFound = 1,
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
  QuotaExceeded,
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
  InProgress,
};

struct Error {
  ErrorKind kind = {};

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  auto as_str() const noexcept -> Str;

  void fmt(auto& f) const {
    const auto s = this->as_str();
    f.write_str(s);
  }

 public:
  auto operator==(decltype(nullptr)) const noexcept -> bool {
    return kind != ErrorKind{};
  }
};

template <class T = void>
using Result = result::Result<T, Error>;

}  // namespace sfc::io
