#include "rc.inl"

#include "rc/io/mod.h"

#include "rc/os.h"
#include "rc/sys.h"

namespace rc::io {

pub auto Error::last_os_error() -> Error {
  const auto code = os::last_error();
  return Error{code};
}

pub auto Error::from_raw_os_error(i32 code) -> Error {
  const auto res = Error{code};
  return res;
}

pub auto Error::kind() const noexcept -> ErrorKind {
  const auto x = sys::io::get_err_kind(_code);
  return ErrorKind{x};
}

pub auto Error::name() const noexcept -> Str {
  const auto k = this->kind();

#define impl_err(VALUE)  \
  case ErrorKind::VALUE: \
    return u8## #VALUE

  switch (k) {
    impl_err(NotFound);
    impl_err(PermissionDenied);
    impl_err(ConnectionRefused);
    impl_err(ConnectionReset);
    impl_err(ConnectionAborted);
    impl_err(NotConnected);
    impl_err(AddrInUse);
    impl_err(AddrNotAvailable);
    impl_err(BrokenPipe);
    impl_err(AlreadyExists);
    impl_err(WouldBlock);
    impl_err(InvalidInput);
    impl_err(InvalidData);
    impl_err(TimedOut);
    impl_err(WriteZero);
    impl_err(Interrupted);
    impl_err(UnexpectedEof);
    default:
      return u8"Other";
  }
#undef impl_err
}

}  // namespace rc::io
