#pragma once

#include <nms/core.h>
#include <nms/io/path.h>

namespace nms
{
template<class T, u32 BookSize, u32 PageSize>
class ArrayList;
}

namespace nms::math
{
template<class T, u32 N>
class Array;
}

namespace nms::io
{

class File: public INocopyable
{
public:
    enum OpenMode
    {
        Read    = 0x0,      /// open for read, file must exists
        Write   = 0x1,      /// open for write, file will trunk.
        Append  = 0x2,      /// open for append
    };

    class Exception: public IException
    {};

    class ENotEnough: public Exception
    {};

    NMS_API File(const Path& path, OpenMode mode);
    NMS_API ~File();

    File(File&& rhs) noexcept
        : obj_(rhs.obj_)
    {
        rhs.obj_ = nullptr;
    }

    File& operator=(File&& rhs) noexcept {
        nms::swap(obj_, rhs.obj_);
        return *this;
    }

    NMS_API u64 size() const;

#pragma region read/write

#pragma region raw
    NMS_API void sync() const;


    u64 read(void* data, u64 size) {
        return readRaw(data, 1, size);
    }

    u64 write(const void* buff, u64 size) {
        return writeRaw(buff, 1, size);
    }

#pragma endregion

#pragma region array/vec
    /* read data */
    template<class T>
    u64 read(T data[], u64 count) {
        auto nread = readRaw(static_cast<void*>(data), sizeof(T), count);
        return nread;
    }

    /* write data */
    template<class T>
    u64 write(const T data[], u64 count) {
        auto nwrite = writeRaw(data, sizeof(T), count);
        return nwrite;
    }

    template<class T, u32 N>
    u64 read(T(&data)[N]) {
        return read(data, N);
    }

    template<class T, u32 N>
    u64 write(const T(&data)[N]) {
        return write(data, N);
    }

#pragma endregion

#pragma region view<T,N>
    /* read view data */
    template<class T, u32 N>
    void read(View<T, N>& view) {
        if (!view.isNormal()) {
            throw_exception(EBadType{});
        }

        read(view.data(), view.count());
    }

    /* write view data */
    template<class T, u32 N>
    void write(const View<T, N>& view) {
        if (!view.isNormal()) {
            throw_exception(EBadType{});
        }
        write(view.data(), view.count());
    }

#pragma region save/load
    template<class T, class = $when_is<$pod, T>>
    File& operator>>(T& val) {
        read(&val, 1);
        return *this;
    }

    template<class T, class = $when_is<$pod, T>>
    File& operator<<(const T& val) {
        write(&val, 1);
        return *this;
    }

    template<class T, u32 N>
    File& operator>>(Vec<T, N>& v) {
        read(v.data(), N);
        return *this;
    }

    template<class T, u32 N>
    File& operator<<(const Vec<T, N>& v) {
        write(v.data(), N);
        return *this;
    }

#pragma endregion

protected:
#ifndef NMS_BUILD
    struct fid_t;
#else
    using fid_t = ::FILE;
#endif

    fid_t*  obj_;   // the FILE* object

    NMS_API u64 readRaw (void*       buffer, u64 size, u64 count);
    NMS_API u64 writeRaw(const void* buffer, u64 size, u64 count);
};

class TxtFile
    : protected File
{
protected:
    using base = File;

public:
    NMS_API TxtFile(const Path& path, File::OpenMode mode);
    NMS_API ~TxtFile();

    using base::OpenMode;
    using base::size;
    using base::sync;

    u64 read(char* u8_buf, u64 size) {
        return _read(u8_buf, size);
    }

    u64 write(StrView u8_str) {
        return _write(u8_str.data(), u8_str.count());
    }

    template<class ...U>
    u64 write(StrView fmt, const U& ...args) {
        auto s = format(fmt, args...);
        return write(s);
    }

private:
    NMS_API u64 _read(char* u8_buf, u64 size);
    NMS_API u64 _write(const char* u8_buf, u64 size);
};

NMS_API String loadString(const Path& path);

}

namespace nms
{
template<class T, u32 BS>
void List<T,BS>::save(io::File& os) const {
    const auto info = base::typeinfo();
    const auto size = base::size();
    os << info;
    os << size;

    os.write(this->data(), this->count());
}

template<class T, u32 BS>
void List<T, BS>::save(const io::Path& path) const {
    io::File file(path, io::File::Write);
    save(file);
}

template<class T, u32 BS>
List<T,BS> List<T, BS>::load(io::File& is) {
    u8x4        info;
    Vec<u32, 1> size;

    is >> info;
    is >> size;
    if (info != base::typeinfo()) {
        throw_exception(EBadType{});
    }

    List<T,BS> tmp(size[0]);
    is.read(tmp.data(), tmp.count());
    return tmp;
}

template<class T, u32 BS>
List<T,BS> List<T, BS>::load(const io::Path& path) {
    io::File file(path, io::File::Read);
    auto ret = load(file);
    return ret;
}

}