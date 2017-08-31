#include <nms/core.h>
#include <nms/test.h>
#include <nms/io/file.h>
#include <nms/io/log.h>
#include <nms/math.h>

namespace nms::io
{

NMS_API File::File(const Path& path, OpenMode mode) {
    const auto cpath = path.cstr();

    auto smod = "rb";
    switch (u32(mode)) {
    case Read:      smod = "rb"; break;
    case Write:     smod = "wb"; break;
    case Append:    smod = "ab"; break;
    default: break;
    }

    obj_            = ::fopen(cpath, smod);
    const auto eid  = errno;
    
    if (obj_ == nullptr) {
        auto dir = cwd();
        log::error("nms.io.File: fopen failed\n"
            "    dir : {}\n"
            "    path: {}", dir, path);
        NMS_THROW(ESystem{eid});
    }
}

NMS_API File::~File() {
    if (obj_ == nullptr) {
        return;
    }
    ::fclose(obj_);
    obj_ = nullptr;
}

NMS_API int File::id() const {
#ifdef NMS_OS_WINDOWS
    auto fid = ::_fileno(obj_);
#else
    auto fid = ::fileno(obj_);
#endif
    return fid;
}

NMS_API u64 File::size() const {
    auto fid = fileno(obj_);
    return fsize(fid);
}

NMS_API u64 File::readRaw(void* dat, u64 size, u64 n) {
    if (obj_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fread(dat, size, n, obj_);
    return ret;
}

NMS_API u64 File::writeRaw(const void* dat, u64 size, u64 n) {
    if (obj_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fwrite(dat, size, n, obj_);
    return ret;
}


NMS_API void File::sync() const {
    fflush(obj_);
}

#pragma region TxtFile
NMS_API TxtFile::TxtFile(const Path& path, File::OpenMode mode)
    : base{ path, OpenMode(mode) }
{
    static const Vec<char, 3> bom = { '\xEF', '\xBB', '\xBF' };

#ifdef NMS_OS_WINDOWS
    auto fid = id();
    _setmode(fid, _O_TEXT);
#endif
    if (mode == File::OpenMode::Read) {
        Vec<char, 3> buf;
        auto cnt = ::fread(buf.data_, 1, 3, obj_);
        if (cnt == 3 && bom == buf) {
            // UTF-8 BOM
        }
        else {
            fseek(obj_, 0, SEEK_SET);
        }
    }
}

NMS_API TxtFile::~TxtFile()
{}

NMS_API u64 TxtFile::_read(char* u8_buf, u64 size) {
    auto ret = base::read(u8_buf, size);
    return ret;
}

NMS_API u64 TxtFile::_write(const char* u8_buf, u64 size) {
    auto ret = base::write(u8_buf, size);
    return ret;
}
#pragma endregion

NMS_API String loadString(const Path& path) {
    TxtFile file(path, File::Read);
    const auto len = u32(file.size());

    String str;
    str.reserve(len+1);

    const auto dat = str.data();
    const auto cnt = file.read(dat, len);
    str.resize(u32(cnt));
    str.cstr();

    return str;
}


NMS_API u64 fsize(const Path& path) {
    auto cpath = path.cstr();
    ::stat_t st;
    ::stat(cpath, &st);
    return st.st_size;
}

NMS_API u64 fsize(int fid) {
    ::stat_t st;
    ::fstat(fid, &st);
    return st.st_size;
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
