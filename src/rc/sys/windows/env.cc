#include "rc/sys/windows.inl"

#include "rc/ffi.h"
#include "rc/io.h"

namespace rc::sys::windows::env {

static auto valid_path(Slice<u8> v) -> void {
  for (auto i = 0u; i < v._len; ++i) {
    if (v[i] == u8'\\') {
      v[i] = u8'/';
    }
  }
}

auto var(Str key) -> String {
  const auto ckey = ffi::CString{key};
  const auto nret = ::GetEnvironmentVariableA(ckey, nullptr, 0);
  if (nret == 0) {
    return {};
  }

  Vec<u8> buf;
  buf.reserve(nret);
  buf._len = nret;
  const auto p = ptr::cast<char>(buf.as_mut_ptr());
  ::GetEnvironmentVariableA(ckey, p, nret);
  env::valid_path(buf.as_mut_slice());
  return String{rc::move(buf)};
}

auto home_dir() -> String {
  auto res = env::var("USERPROFILE");
  return res;
}

auto current_dir() -> String {
  const auto cnt = ::GetCurrentDirectoryA(0, nullptr);

  Vec<u8> buf;
  buf.reserve(cnt);
  buf._len = cnt;
  ::GetCurrentDirectoryA(cnt, ptr::cast<char>(buf.as_mut_ptr()));
  env::valid_path(buf.as_mut_slice());
  return String{rc::move(buf)};
}

auto current_exe() -> String {
  const auto nret = ::GetModuleFileNameA(nullptr, nullptr, 0);

  Vec<u8> buf;
  buf.reserve(nret);
  buf._len = nret;
  ::GetModuleFileNameA(nullptr, ptr::cast<char>(buf.as_mut_ptr()), nret);
  return String{rc::move(buf)};
}

auto set_current_dir(Str path) -> void {
  const auto c_p = ffi::CString{path};
  const auto ret = ::SetCurrentDirectoryA(c_p);
  if (ret == FALSE) {
    throw rc::io::Error::last_os_error();
  }
}

}  // namespace rc::env
