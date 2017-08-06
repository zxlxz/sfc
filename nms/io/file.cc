#include <nms/core.h>
#include <nms/test.h>
#include <nms/io/file.h>
#include <nms/io/log.h>
#include <nms/math.h>

namespace nms::io
{

static constexpr auto TxtMode  = 0x10;
static constexpr auto ReadTxt  = TxtMode | File::Read;
static constexpr auto WriteTxt = TxtMode | File::Write;
static constexpr auto AppendTxt= TxtMode | File::Append;

NMS_API File::File(const Path& path, OpenMode mode) {
    const auto cpath = path.cstr();

    auto smod = "rb+";
    switch (u32(mode)) {
    case Read:      smod = "rb+"; break;
    case Write:     smod = "wb+"; break;
    case Append:    smod = "ab+"; break;

    case ReadTxt:   smod = "rt+"; break;
    case WriteTxt:  smod = "wt+"; break;
    case AppendTxt: smod = "at+"; break;

    default: break;
    }

    fid_            = ::fopen(cpath, smod);
    const auto eid  = errno;
    
    if (fid_ == nullptr) {
        auto dir = cwd();
        log::error("nms.io.File: fopen failed\n"
            "    dir : {}\n"
            "    path: {}", dir, path);
        throw ESystem{eid};
    }
}

NMS_API File::~File() {
    if (fid_ == nullptr) {
        return;
    }
    ::fclose(fid_);
    fid_ = nullptr;
}

NMS_API u64 File::size() const {
    constexpr auto seek_set = 0;
    constexpr auto seek_cur = 1;    (void)seek_cur;
    constexpr auto seek_end = 2;
#ifdef NMS_OS_WINDOWS
    const auto cur = _ftelli64(fid_);
    const auto end = _fseeki64(fid_, 0, seek_end);      (void)end;
    const auto len = _ftelli64(fid_);
    const auto ret = _fseeki64(fid_, cur, seek_set);    (void)ret;
#else
    const auto cur = ftell(fid_);
    const auto end = fseek(fid_, 0, seek_end);          (void)end;
    const auto len = ftell(fid_);
    const auto ret = fseek(fid_, cur, seek_end);        (void)ret;
#endif
    return u64(len);
}

NMS_API u64 File::readRaw(void* dat, u64 size, u64 n) {
    if (fid_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fread(dat, size, n, fid_);
    return ret;
}

NMS_API u64 File::writeRaw(const void* dat, u64 size, u64 n) {
    if (fid_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fwrite(dat, size, n, fid_);
    return ret;
}

NMS_API TxtFile::TxtFile(const Path& path, File::OpenMode mode)
    : base{ path, OpenMode(TxtMode | mode) }
{}

NMS_API TxtFile::~TxtFile()
{}

NMS_API void File::sync() const {
    fflush(fid_);
}

NMS_API String loadString(const Path& path) {
    TxtFile file(path, TxtFile::Read);
    const auto len = u32(file.size());

    String str;
    str.reserve(len+1);

    const auto dat = str.data();
    const auto cnt = file.read(dat, len);
    str.resize(u32(cnt));
    str.cstr();

    return str;
}

#pragma region unittest
nms_test(file) {
    Array<i8, 2> a({ 256u, 4u });
    a <<= vlins<i8>(1, 1);
    File file("nms.io.file.dat", File::Write);
    file.write(a);
}
#pragma endregion

}
