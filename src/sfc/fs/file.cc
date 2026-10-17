#include "sfc/fs/file.h"
#include "sfc/sys/io.h"
#include "sfc/sys/fs.h"
#include "sfc/ffi/os_str.h"

namespace sfc::fs {

using ffi::OsString;

auto Metadata::file_len() const noexcept -> u64 {
  return _size;
}

auto Metadata::exists() const noexcept -> bool {
  return sys::FileAttr{_attr}.exists();
}

auto Metadata::is_dir() const noexcept -> bool {
  return sys::FileAttr{_attr}.is_dir();
}

auto Metadata::is_file() const noexcept -> bool {
  return sys::FileAttr{_attr}.is_file();
}

auto Metadata::is_link() const noexcept -> bool {
  return sys::FileAttr{_attr}.is_link();
}

auto OpenOptions::open(Path path) const noexcept -> io::Result<File> {
  const auto os_path = OsString::from_str(path.as_str());
  const auto fd = _TRY(sys::open(os_path.as_ptr(), *this));
  return {File::from_raw_fd(fd)};
}

File::File(io::File other) noexcept : io::File{mem::move(other)} {}

File::~File() = default;

File::File(File&&) noexcept = default;

auto File::operator=(File&&) noexcept -> File& = default;

auto File::from_raw_fd(io::RawFd fd) -> File {
  auto res = File{io::File::from_raw_fd(fd)};
  return res;
}

auto File::open(Path path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{.read = true, .write = true};
  return opts.open(path);
}

auto File::create(Path path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{.create = true, .write = true, .truncate = true};
  return opts.open(path);
}

auto File::metadata() noexcept -> io::Result<fs::Metadata> {
  const auto fd = this->as_raw_fd();
  return sys::fstat(fd);
}

struct ReadDir::Inn : sys::ReadDir {};

ReadDir::ReadDir(Inn& inn) noexcept : _inn{&inn} {}

ReadDir::~ReadDir() {
  if (_inn == nullptr) {
    return;
  }
  (void)_inn->close();
  delete _inn;
}

ReadDir::ReadDir(ReadDir&& other) noexcept : _inn{mem::take(other._inn)} {}

ReadDir& ReadDir::operator=(ReadDir&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto ReadDir::next() -> Option<DirEntry> {
  if (_inn == nullptr) {
    return {};
  }

  const auto sys_entry = _inn->next();
  if (!sys_entry) {
    return {};
  }

  const auto os_path = ffi::OsStr{sys_entry->path};
  const auto fs_meta = fs::Metadata{
      ._attr = u32(sys_entry->attr),
      ._size = u64(sys_entry->size),
  };

  auto dir_entry = DirEntry{
      .path = PathBuf::from(os_path.to_string()),
      .meta = mem::move(fs_meta),
  };
  return dir_entry;
}

auto ReadDir::open(Path path) -> io::Result<ReadDir> {
  const auto os_path = OsString::from_str(path.as_str());

  auto sys = _TRY(sys::ReadDir::open(os_path.as_ptr()));
  auto inn = new Inn{sys};
  return ReadDir{*inn};
}

auto read(Path path) noexcept -> io::Result<List<u8>> {
  auto file = _TRY(File::open(path));

  auto buf = List<u8>{};
  _TRY(file.read_to_end(buf));
  return mem::move(buf);
}

auto write(Path path, Slice<const u8> buf) noexcept -> io::Result<> {
  auto file = _TRY(File::create(path));

  _TRY(file.write_all(buf));
  return Ok{};
}

auto create_dir(Path path) -> io::Result<> {
  if (path._inn.is_empty() || path.is_root()) {
    return {io::Error::InvalidInput};
  }

  const auto os_path = OsString::from_str(path.as_str());
  return sys::mkdir(os_path.as_ptr());
}

auto create_dir_all(Path path) -> io::Result<> {
  if (path.as_str() == "" || path.is_root()) {
    return Ok{};
  }

  const auto err = fs::create_dir(path).err();
  if (!err || *err == io::Error::AlreadyExists) {
    return Ok{};
  }

  const auto parent = path.parent();
  _TRY(fs::create_dir_all(parent));

  return fs::create_dir(path);
}

auto remove_dir(Path path) -> io::Result<> {
  const auto os_path = OsString::from_str(path.as_str());
  return sys::rmdir(os_path.as_ptr());
}

auto remove_file(Path path) -> io::Result<> {
  const auto os_path = OsString::from_str(path.as_str());
  return sys::unlink(os_path.as_ptr());
}

auto rename(Path old_path, Path new_path) -> io::Result<> {
  const auto os_old = OsString::from_str(old_path.as_str());
  const auto os_new = OsString::from_str(new_path.as_str());
  return sys::rename(os_old.as_ptr(), os_new.as_ptr());
}

auto metadata(Path path) -> io::Result<Metadata> {
  const auto os_path = OsString::from_str(path.as_str());
  const auto meta = _TRY(sys::lstat(os_path.as_ptr()));
  return {meta};
}

auto read_dir(Path path) -> io::Result<ReadDir> {
  return ReadDir::open(path);
}

}  // namespace sfc::fs
