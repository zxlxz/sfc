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

    auto smod = "rb";
    switch (u32(mode)) {
    case Read:      smod = "rb"; break;
    case Write:     smod = "wb"; break;
    case Append:    smod = "ab"; break;

    case ReadTxt:   smod = "r"; break;
    case WriteTxt:  smod = "w"; break;
    case AppendTxt: smod = "a"; break;

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

NMS_API TxtFile::TxtFile(const Path& path, File::OpenMode mode)
    : base{ path, OpenMode(TxtMode | mode) }
{}

NMS_API TxtFile::~TxtFile()
{}

NMS_API void File::sync() const {
    fflush(obj_);
}

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

#pragma region unittest
nms_test(file) {
    Array<i8, 2> a({ 256u, 4u });
    a <<= vlins<i8>(1, 1);
    File file("nms.io.file.dat", File::Write);
    file.write(a);
}
#pragma endregion

}
