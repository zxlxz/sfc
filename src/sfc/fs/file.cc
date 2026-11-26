#include "sfc/fs/file.h"
#include "sfc/fs/path.h"
#include "sfc/sys/io.h"
#include "sfc/sys/fs.h"

namespace sfc::fs {

File::File(io::File file) noexcept : _inn{mem::move(file)} {}

File::~File() {
  (void)_inn.close();
}

File::File(File&& other) noexcept : _inn{mem::take(other._inn)} {}

auto File::operator=(File&& other) noexcept -> File& {
  if (this != &other) {
    _inn = mem::take(other._inn);
  }
  return *this;
}

auto File::from_raw_fd(io::RawFd fd) -> File {
  auto res = File{io::File{fd}};
  return res;
}

auto File::open(Str path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{.read = true, .write = false};
  return opts.open(path);
}

auto File::create(Str path) noexcept -> io::Result<File> {
  const auto opts = OpenOptions{.create = true, .write = true, .truncate = true};
  return opts.open(path);
}

auto File::read(Slice<u8> buf) noexcept -> io::Result<usize> {
  return _inn.read(buf);
}

auto File::write(Slice<const u8> buf) noexcept -> io::Result<usize> {
  return _inn.write(buf);
}

auto File::flush() noexcept -> io::Result<> {
  return _inn.flush();
}

auto File::seek(io::SeekFrom pos) noexcept -> io::Result<u64> {
  return _inn.seek(pos);
}

auto File::metadata() noexcept -> io::Result<fs::Metadata> {
  return sys::fstat(_inn._fd);
}

auto OpenOptions::open(Str path) const noexcept -> io::Result<File> {
  const auto fd = _TRY(sys::open(path, *this));
  return {File::from_raw_fd(fd)};
}

auto Metadata::file_size() const noexcept -> u64 {
  return _size;
}

auto Metadata::exists() const noexcept -> bool {
  return _attr != 0;
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

  const auto entry_opt = _inn->next();
  if (!entry_opt) {
    return {};
  }

  const auto& entry = *entry_opt;
  return DirEntry{
      .path = Str::from_cstr(entry._name),
      .meta = {._size = entry._size, ._attr = entry._attr},
  };
}

auto ReadDir::open(Str path) -> io::Result<ReadDir> {
  auto sys = _TRY(sys::ReadDir::open(path));
  auto inn = new Inn{sys};
  return ReadDir{*inn};
}

auto read(Str path) noexcept -> io::Result<List<u8>> {
  auto file = _TRY(File::open(path));

  auto buf = List<u8>::new_();
  _TRY(file.read_to_end(buf));
  return mem::move(buf);
}

auto write(Str path, Slice<const u8> buf) noexcept -> io::Result<> {
  auto file = _TRY(File::create(path));
  _TRY(file.write_all(buf));
  return Ok{};
}

auto create_dir(Str path) -> io::Result<> {
  if (path.is_empty()) {
    return io::Error::InvalidInput;
  }

  return sys::mkdir(path);
}

auto create_dir_all(Str path) -> io::Result<> {
  if (path.is_empty()) {
    return Ok{};
  }

  const auto err = fs::create_dir(path).err();
  if (!err || *err == io::Error::AlreadyExists) {
    return Ok{};
  }

  const auto parent = Path{path}.parent();
  if (parent.as_str().is_empty()) {
    return *err;
  }

  _TRY(fs::create_dir_all(parent.as_str()));

  return fs::create_dir(path);
}

auto remove_dir(Str path) -> io::Result<> {
  return sys::rmdir(path);
}

auto remove_file(Str path) -> io::Result<> {
  return sys::unlink(path);
}

auto rename(Str old_path, Str new_path) -> io::Result<> {
  return sys::rename(old_path, new_path);
}

auto metadata(Str path) -> io::Result<Metadata> {
  const auto meta = _TRY(sys::lstat(path));
  return {meta};
}

auto read_dir(Str path) -> io::Result<ReadDir> {
  return ReadDir::open(path);
}

}  // namespace sfc::fs
