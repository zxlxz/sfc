#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

enum class ErrorKind {
  NotFound,
  PermissionDenied,
  ConnectionRefused,
  ConnectionReset,
  ConnectionAborted,
  NotConnected,
  AddrInUse,
  AddrNotAvailable,
  BrokenPipe,
  AlreadyExists,
  WouldBlock,
  InvalidInput,
  InvalidData,
  TimedOut,
  WriteZero,
  UnexpectedEof,
  Interrupted,
  Other,
};

auto to_str(ErrorKind kind) -> Str;

struct Error {
  ErrorKind kind = ErrorKind::Other;
  int code = 0;

 public:
  static auto last_os_error() -> Error;

  static auto from_os_error(int code) -> Error;

  void fmt(auto& f) const {
    const auto s = to_str(kind);
    f.write_str(s);
  }
};

template <class T = Tuple<>>
using Result = result::Result<T, Error>;

struct Read {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto cnt = _TRY(Result{self.read(buf)});
      if (cnt == 0) {
        return Error{ErrorKind::UnexpectedEof, 0};
      }
      buf = buf.slice(cnt);
    }

    return {};
  }

  auto read_to_end(this auto& self, Vec<u8>& buf, usize buf_len = 256) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      buf.reserve(buf_len);

      auto tmp = Slice{buf.as_mut_ptr() + buf.len(), buf_len};
      const auto cnt = _TRY(Result{self.read(tmp)});
      if (cnt == 0) {
        break;
      }
      buf.set_len(buf.len() + cnt);
    }
    return buf.len() - old_len;
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return self.read_to_end(buf.as_mut_vec());
  }
};

struct Write {
  auto write_all(this auto& self, Slice<const u8> buf) -> Result<> {
    static_assert(requires { self.write(buf); });

    while (!buf.is_empty()) {
      const auto cnt = _TRY(Result{self.write(buf)});
      if (cnt == 0) {
        return Error{ErrorKind::WriteZero, 0};
      }
      buf = buf.slice(cnt);
    }
    return {};
  }

  auto write_char(this auto& self, auto c) -> Result<> {
    const u8 buf[] = {u8(c)};
    _TRY(Result{self.write(buf)});
    return {};
  }

  auto write_str(this auto& self, Str buf) -> Result<> {
    return self.write_all(buf.as_bytes());
  }
};

}  // namespace sfc::io
