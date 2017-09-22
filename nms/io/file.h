#pragma once

#include <nms/core.h>
#include <nms/io/path.h>

namespace nms
{
template<class T, u32 Ibooksize, u32 Ipagesize>
class ArrayList;
}

namespace nms::math
{
template<class T, u32 N>
class Array;
}

namespace nms::io
{

class File
{
public:
    enum OpenMode
    {
        Read    = 0x0,      /// open for read, file must exists
        Write   = 0x1,      /// open for write, file will trunk.
        Append  = 0x2,      /// open for append
    };

    enum FileType
    {
        Binary,
        Text
    };

    class Exception: public IException
    {};

    class ENotEnough: public Exception
    {};

    constexpr File()
        : impl_(nullptr)
    {}

    File(const Path& path, OpenMode mode) {
        open(path, mode);
    }

    virtual ~File() {
        close();
    }

    File(File&& rhs) noexcept
        : impl_(rhs.impl_)
    {
        rhs.impl_ = nullptr;
    }

    File(const File&) = delete;

    File& operator=(File&& rhs) noexcept {
        nms::swap(impl_, rhs.impl_);
        return *this;
    }

    File& operator=(const File&) = delete;

    operator bool() const {
        return impl_ != nullptr;
    }

    /*! open file */
    NMS_API virtual void open(const Path& path, OpenMode mode);

    /*! close file */
    NMS_API void close();

    /*! get file size in bytes */
    NMS_API u64 size() const;

    /*! get virtual-file-system integer descriptor */
    NMS_API int id()   const;

#pragma region read/write

#pragma region raw
    /*! flush the file stream */
    NMS_API void sync() const;

    /*! flush the file stream */
    NMS_API void flush() const;

    /*! read @size bytes from the file into the buffer start at @data */
    u64 read(void* data, u64 size) const {
        if (impl_ == nullptr) {
            return 0;
        }
        return readRaw(data, 1, size);
    }

    /*! write @size bytes from the file from the buffer start at @data */
    u64 write(const void* buff, u64 size) {
        if (impl_ == nullptr) {
            return 0;
        }
        return writeRaw(buff, 1, size);
    }

#pragma endregion

#pragma region array/vec
    /* read data */
    template<class T>
    u64 read(T data[], u64 count) const {
        if (impl_ == nullptr) {
            return 0;
        }
        u64 nread = readRaw(static_cast<void*>(data), sizeof(T), count);
        return nread;
    }

    /* write data */
    template<class T>
    u64 write(const T data[], u64 count) {
        if (impl_ == nullptr) {
            return 0;
        }
        u64 nwrite = writeRaw(data, sizeof(T), count);
        return nwrite;
    }
#pragma endregion

protected:
#ifndef NMS_BUILD
    struct fid_t;
#else
    using fid_t = ::FILE;
#endif
    fid_t*  impl_ = nullptr;   // the FILE* object

    NMS_API u64 readRaw (void*       buffer, u64 size, u64 count) const;
    NMS_API u64 writeRaw(const void* buffer, u64 size, u64 count);
};

using DatFile = File;
using BinFile = File;

class TxtFile
    : public File
{
protected:
    using base = File;

public:
    constexpr TxtFile()
    { }

    TxtFile(const Path& path, File::OpenMode mode) {
        open(path, mode);
    }

    virtual ~TxtFile()
    { }

    TxtFile(const TxtFile&)             = delete;
    TxtFile& operator= (const TxtFile&) = delete;

    NMS_API void open(const Path& path, File::OpenMode mode) override;

    u64 read(IString& u8_str, u64 size) const {
        const auto oldcnt = u8_str.count();
        const auto newcap = oldcnt + u32(size);
        u8_str.reserve(newcap);

        const auto ret  = _read(u8_str.data() + oldcnt, size);
        const auto newcnt = oldcnt + u32(ret);
        u8_str._resize(newcnt);

        return ret;
    }

    u64 readall(IString& u8_str) {
        const auto file_size = size();
        return read(u8_str, file_size);
    }

    u64 write(const StrView& u8_str) {
        return _write(u8_str.data(), u8_str.count());
    }

    template<class ...U>
    u64 write(const StrView& fmt, const U& ...args) {
        auto s = format(fmt, args...);
        return write(s);
    }

private:
    NMS_API u64 _read(char* u8_buf, u64 size) const;
    NMS_API u64 _write(const char* u8_buf, u64 size);
};

NMS_API u64   fsize(const Path& path);
NMS_API u64   fsize(int fid);

}
