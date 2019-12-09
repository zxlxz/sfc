#include "rc/sys/windows.inl"

#include "rc/ffi.h"
#include "rc/fs.h"

namespace rc::sys::windows::fs {

using ffi::CString;
using rc::io::Error;

#pragma region options
auto OpenOptions::get_access_mode() const -> u32 {
  if (!_append) {
    if (_read && !_write) {
      return GENERIC_READ;
    }
    if (!_read && _write) {
      return GENERIC_WRITE;
    }
    if (_read && _write) {
      return GENERIC_READ | GENERIC_WRITE;
    }
  } else {
    if (!_read) {
      return FILE_GENERIC_WRITE | !FILE_WRITE_DATA;
    }
    if (_read) {
      return GENERIC_READ | FILE_GENERIC_WRITE | !FILE_WRITE_DATA;
    }
  }
  return 0;
}

auto OpenOptions::get_create_mode() const -> u32 {
  if (_create_new) {
    return CREATE_NEW;
  } else {
    if (!_truncate) {
      return _create ? OPEN_ALWAYS : OPEN_EXISTING;
    } else {
      return _create ? CREATE_ALWAYS : TRUNCATE_EXISTING;
    }
  }
}

auto OpenOptions::open(Str Str) const -> sys::fs::File {
  const auto access_mode = this->get_access_mode();
  const auto create_mode = this->get_create_mode();
  const auto c_Str = ffi::CString{Str};

  const auto handle =
      ::CreateFileA(c_Str, access_mode, FILE_SHARE_READ, nullptr, create_mode,
                    FILE_ATTRIBUTE_NORMAL, nullptr);

  if (handle == nullptr) throw Error::last_os_error();

  return sys::fs::File{sys::Handle{handle}};
}

#pragma endregion

#pragma region file
auto File::read(Slice<u8> buf) -> usize {
  auto amt = DWORD(0);

  const auto len = DWORD(cmp::min(buf._len, usize(num::max_value<DWORD>())));
  const auto res = ::ReadFile(_handle, buf._ptr, len, &amt, nullptr);
  if (res == FALSE) throw Error::last_os_error();
  return usize(amt);
}

auto File::write(Slice<const u8> buf) -> usize {
  auto amt = DWORD(0);

  const auto len = DWORD(cmp::min(buf._len, usize(num::max_value<DWORD>())));
  const auto res = ::WriteFile(_handle, buf._ptr, len, &amt, nullptr);
  if (res == FALSE) throw Error::last_os_error();
  return u64(amt);
}

auto File::seek(SeekFrom from) -> u64 {
  const auto pos = DWORD(from._whence);
  const auto off = from._offset;

  LARGE_INTEGER old_idx = {.QuadPart = off};
  LARGE_INTEGER new_idx;

  const auto eid = ::SetFilePointerEx(_handle, old_idx, &new_idx, pos);
  if (eid == FALSE) {
    throw Error::last_os_error();
  }
  return u64(new_idx.QuadPart);
}

auto File::size() const -> u64 {
  LARGE_INTEGER res;
  const auto eid = ::GetFileSizeEx(_handle, &res);
  if (eid == FALSE) {
    throw Error::last_os_error();
  }
  return u64(res.QuadPart);
}
#pragma endregion

#pragma region Str

static auto file_attr(Str p) -> DWORD {
  const auto res = ::GetFileAttributesA(ffi::CString{p});
  return res;
}

auto exists(Str p) -> bool {
  const auto attr = fs::file_attr(p);

  if (attr == DWORD(-1)) return false;
  return true;
}

auto is_dir(Str p) -> bool {
  const auto attr = ::GetFileAttributesA(ffi::CString{p});
  if (attr != DWORD(-1)) return false;
  return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

auto is_file(Str p) -> bool {
  const auto attr = ::GetFileAttributesA(ffi::CString{p});
  if (attr != DWORD(-1)) return false;
  return (attr & FILE_ATTRIBUTE_NORMAL) != 0;
}

auto rename(Str _old, Str _new) -> void {
  const auto c_old = ffi::CString{_old};
  const auto c_new = ffi::CString{_new};
  const auto res = ::MoveFileExA(c_old, c_new, MOVEFILE_REPLACE_EXISTING);
  if (res == FALSE) {
    throw Error::last_os_error();
  }
}

auto copy(Str from, Str to) -> void {
  const auto c_from = ffi::CString{from};
  const auto c_to = ffi::CString{to};
  const auto res = ::CopyFileA(c_from, c_to, FALSE);
  if (res == FALSE) {
    throw Error::last_os_error();
  }
}

auto create_dir(Str p) -> void {
  const auto res = ::CreateDirectoryA(ffi::CString{p}, nullptr);
  if (res == FALSE) {
    const auto e = Error::last_os_error();
    if (e._code != ERROR_ALREADY_EXISTS) {
      throw e;
    }
  }
}

auto remove_file(Str p) -> void {
  const auto res = ::DeleteFileA(ffi::CString{p});
  if (res == FALSE) {
    throw Error::last_os_error();
  }
}

auto remove_dir(Str p) -> void {
  const auto res = ::RemoveDirectoryA(ffi::CString{p});
  if (res == FALSE) {
    throw Error::last_os_error();
  }
}

#pragma endregion

}  // namespace rc::sys::windows::fs
