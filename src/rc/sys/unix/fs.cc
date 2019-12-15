#include "rc/sys/unix.inl"

#include "rc/ffi.h"
#include "rc/fs.h"

namespace rc::sys::unix::fs {

#pragma region OpenOptions
auto OpenOptions::get_access_mode() const -> u32 {
  if (!_append) {
    if (_read && !_write) {
      return O_RDONLY;
    }
    if (!_read && _write) {
      return O_WRONLY;
    }
    if (_read && _write) {
      return O_RDWR;
    }
  }
  if (!_read) {
    return O_WRONLY | O_APPEND;
  } else {
    return O_RDWR | O_APPEND;
  }
}

auto OpenOptions::get_create_mode() const -> u32 {
  if (_create_new) {
    return O_CREAT | O_EXCL;
  }
  if (!_truncate) {
    return _create ? O_CREAT : 0;
  } else {
    return _create ? (O_CREAT | O_TRUNC) : O_TRUNC;
  }
}

auto OpenOptions::open(Str path) const -> File {
  const auto flags = O_CLOEXEC | get_access_mode() | get_create_mode();
  const auto mode = 0666;
  const auto c_path = ffi::CString{path};
  const auto fid = ::open(c_path, flags, mode);
  if (fid == -1) {
    throw io::Error::last_os_error();
  }
  return File{FileDesc(fid)};
}

#pragma endregion

#pragma region File

auto File::read(Slice<u8> buf) -> usize {
  const auto res = ::read(_filedesc, buf._ptr, buf._len);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return usize(res);
}

auto File::write(Slice<const u8> buf) -> usize {
  const auto res = ::write(_filedesc, buf._ptr, buf._len);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return usize(res);
}

auto File::seek(SeekFrom pos) -> usize {
  const auto whence = [=]() -> int {
    switch (pos._whence) {
      case SeekFrom::Tag::Start:
        return SEEK_SET;
      case SeekFrom::Tag::Current:
        return SEEK_CUR;
      case SeekFrom::Tag::End:
        return SEEK_END;
    }
    return SEEK_SET;
  }();

#ifdef __APPLE__
  const auto res = ::lseek(_filedesc, pos._offset, whence);
#else
  const auto res = ::lseek64(_filedesc, pos._offset, whence);
#endif

  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return usize(res);
}

auto File::size() const -> usize {
  struct ::stat st {};
  const auto res = ::fstat(_filedesc, &st);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return usize(st.st_size);
}

#pragma endregion

#pragma region path

static auto file_attr(Str p) -> ::mode_t {
  struct ::stat st;
  if (::lstat(ffi::CString{p}, &st) != 0) {
    return 0;
  }
  return st.st_mode;
}

auto exists(Str p) -> bool {
  const auto attr = fs::file_attr(p);
  return attr != 0;
}

auto is_dir(Str p) -> bool {
  const auto attr = fs::file_attr(p);
  return (attr & S_IFMT) == S_IFDIR;
}

auto is_file(Str p) -> bool {
  const auto attr = fs::file_attr(p);
  return (attr & S_IFMT) == S_IFREG;
}

auto rename(Str _old, Str _new) -> void {
  const auto c_old = ffi::CString{_old};
  const auto c_new = ffi::CString{_new};
  const auto ret = ::rename(c_old, c_new);
  if (ret == -1) {
    throw io::Error::last_os_error();
  }
}

auto copy(Str from, Str to) -> void {
  const auto c_from = ffi::CString{from};
  const auto c_to = ffi::CString{to};
#ifdef __APPLE__
  const auto res = ::copyfile(c_from, c_to, nullptr, COPYFILE_ALL);
#else
  const auto res = -1;
#endif

  if (res == -1) {
    throw io::Error::last_os_error();
  }
}

auto create_dir(Str p) -> void {
  const auto c_p = ffi::CString{p};
  const auto res = ::mkdir(c_p, 0777);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
}

auto remove_dir(Str p) -> void {
  const auto c_p = ffi::CString{p};
  const auto res = ::rmdir(c_p);

  if (res == -1) {
    throw io::Error::last_os_error();
  }
}

auto remove_file(Str p) -> void {
  const auto c_p = ffi::CString{p};
  const auto res = ::unlink(c_p);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
}

#pragma endregion

}  // namespace rc::sys::unix::fs
