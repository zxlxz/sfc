
#include <nms/test.h>
#include <nms/io/file.h>
#include <nms/io/log.h>

namespace nms::io
{

NMS_API void File::open(const Path& path, OpenMode mode) {
    if (impl_ != nullptr) {
        log::error("nms.io.File: fopen failed, file already opened");
        return;
    }

    auto smod = "rb";
    switch (u32(mode)) {
    case Read:      smod = "rb"; break;
    case Write:     smod = "wb"; break;
    case Append:    smod = "ab"; break;
    default: break;
    }

    const auto real_path = path.path();
    impl_ = ::fopen(real_path.data(), smod);

    if (impl_ == nullptr) {
        auto dir = cwd();
        log::error("nms.io.File: fopen failed\n"
            "    dir : {}\n"
            "    path: {}", dir, path);
        NMS_THROW(ESystem{});
    }
}

NMS_API void File::close() {
    if (impl_ == nullptr) {
        return;
    }
    ::fflush(impl_);
    ::fclose(impl_);
    impl_ = nullptr;
}

NMS_API int File::id() const {
#ifdef NMS_OS_WINDOWS
    auto fid = ::_fileno(impl_);
#else
    auto fid = ::fileno(impl_);
#endif
    return fid;
}

NMS_API u64 File::size() const {
    if (impl_ == nullptr) {
        return 0;
    }

    const auto file_no   = id();
    const auto file_size = fsize(file_no);
    return file_size;
}

NMS_API u64 File::readRaw(void* dat, u64 size, u64 n) const {
    if (impl_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fread(dat, size, n, impl_);
    return ret;
}

NMS_API u64 File::writeRaw(const void* dat, u64 size, u64 n) {
    if (impl_ == nullptr || size == 0 || n == 0) {
        return 0;
    }
    const auto ret = ::fwrite(dat, size, n, impl_);
    return ret;
}


NMS_API void File::sync() const {
    if (impl_ == nullptr) {
        return;
    }

    ::fflush(impl_);
}

NMS_API void File::flush() const {
    if (impl_ == nullptr) {
        return;
    }

    ::fflush(impl_);
}

#pragma region TxtFile
NMS_API void TxtFile::open(const Path& path, File::OpenMode mode) {
    static const Vec<char, 3> utf8_bom = { '\xEF', '\xBB', '\xBF' };

    File::open(path, mode);
#ifdef NMS_OS_WINDOWS
    auto fid = id();
    (void)_setmode(fid, _O_TEXT);
#endif
    if (mode == File::OpenMode::Read) {
        Vec<char, 3> buf;
        auto cnt = ::fread(buf.data, 1, 3, impl_);
        if (cnt == 3 && utf8_bom == buf) {
            // UTF-8 BOM
        }
        else {
            fseek(impl_, 0, SEEK_SET);
        }
    }
}

NMS_API u64 TxtFile::_read(char* u8_buf, u64 size) const {
    if (base::impl_ == nullptr || u8_buf == nullptr || size == 0) {
        return 0;
    }

    auto ret = base::read(u8_buf, size);
    return ret;
}

NMS_API u64 TxtFile::_write(const char* u8_buf, u64 size) {
    if (base::impl_ == nullptr || u8_buf == nullptr || size == 0) {
        return 0;
    }

    auto ret = base::write(u8_buf, size);
    return ret;
}
#pragma endregion

NMS_API u64 fsize(const Path& path) {
    auto path_str = path.path();
    ::stat_t st;
    ::stat(path_str.data(), &st);
    return u64(st.st_size);
}

NMS_API u64 fsize(int fid) {
    ::stat_t st;
    ::fstat(fid, &st);
    return u64(st.st_size);
}


#pragma region unittest
nms_test(file) {
    List<u32, 10> data_in;
    List<u32, 10> data_out;

    const StrView path = "nms.io.file.nums.dat";

    // write
    {
        for (u32 i = 0; i < data_in.$capacity; ++i) {
            data_in += i;
        }

        File file(path, File::Write);
        file.write(data_in.data(), data_in.count());
    }

    // read
    {
        test::assert_true(exists(path));

        File file(path, File::Read);
        test::assert_true(file.size() == data_out.$capacity *sizeof(u32));
        file.read(data_out.data(), data_out.$capacity);
        data_out.resize(data_out.$capacity);

        for (u32 i = 0; i < data_out.$capacity; ++i) {
            test::assert_eq(data_in[i], data_out[i]);
        }
    }

    io::remove(path);

}
#pragma endregion

}
