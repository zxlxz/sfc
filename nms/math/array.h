#pragma once

#include <nms/math/base.h>

namespace nms::io
{
class File;
class Path;
}

namespace nms::math
{

template<class T, u32 N=1>
class Array
    : public View<T, N>
{
public:
    using base  = View<T, N>;
    using Tthis = Array;
    using Tsize = typename base::Tsize;
    using Tdims = typename base::Tdims;
    using Tinfo = typename base::Tinfo;

    static const auto $rank = base::$rank;

#pragma region constructors
    /* default constructor */
    constexpr Array()
        : base{}
    {}

    /* constructor */
    explicit Array(const Tsize(&dims)[$rank])
        : base{ nullptr, dims }
    {
        const auto cnt = this->count();
        if (cnt != 0) {

            // SIMD require memory aligned (128*8=1024)
            // SSE(64bit~128bit), AVX(256bit~512bit)
            this->data_ = anew<T>(cnt, 256);
        }
    }

    /* destructor */
    ~Array() {
        // try delete
        if (this->data_ != nullptr) {
            adel(this->data_);
            this->data_ = nullptr;
        }
    }

    /* move constructor */
    Array(Array&& rhs) noexcept
        : base{ static_cast<base&&>(rhs) }
    {
        rhs.base::operator=(base{});
    }

    /* move assign operator */
    Array& operator=(Array&& rhs) noexcept {
        if (this != &rhs) {
            this->clear();
            this->base::operator=(rhs);
            rhs.base::operator=(base{});
        }
        return *this;
    }

    /* copy assign operator is disabled */
    Array& operator=(const Array& rhs) = delete;

    /* get copies */
    Array dup() const {
        const auto dims = this->size();
        Array tmp(dims);

        const auto cnt = this->count();
        const auto src = this->data();
        const auto dst = tmp.data();
        if (cnt != 0) {
            mcpy(dst, src, cnt);
        }
        return tmp;
    }
#pragma endregion

#pragma region methods
    Array& resize(const Tsize(&newlen)[$rank]) {
        const auto oldlen = this->size();

        if (oldlen != Tdims{ newlen }) {
            Array tmp{ newlen };
            this->operator=(static_cast<Array&&>(tmp));
        }
        return *this;
    }

    Array& clear() {
        // if not empty:
        if (this->data_ != nullptr) {
            adel(this->data_);
            this->base::operator=(base{});
        }
        return *this;
    }
#pragma endregion

#pragma region save/load
    void save(io::File& file) const {
        return this->saveFile(file);
    }

    static auto load(const io::File& file) {
        return Array::loadFile(file);
    }

    void save(const io::Path& path) const {
        return this->savePath<io::File>(path);
    }

    static auto load(const io::Path& path) {
        return Array::loadPath<io::File>(path);
    }
#pragma endregion

protected:
    Array(const Array& rhs)
        : Array{ rhs.size() } {
        *this <<= rhs;
    }

private:
    template<class File>
    void saveFile(File& file) const {
        const typename base::Tinfo info = this->$info;
        const typename base::Tdims size = this->size();

        file.write(&info, 1);
        file.write(&size, 1);
        file.write(this->data(), this->count());
    }

    template<class File>
    static auto loadFile(const File& file) {
        typename base::Tinfo info;
        typename base::Tdims size;

        file.read(&info, 1);
        if (info != base::$info) {
            NMS_THROW(Eunexpect<Tinfo>(base::$info, info));
        }

        file.read(&size, 1);

        Array tmp(size);
        file.read(tmp.data(), tmp.count());
        return tmp;
    }

    template<class File, class Path>
    void savePath(const Path& path) const {
        File file(path, File::Write);
        saveFile(file);
    }

    template<class File, class Path>
    static auto loadPath(const Path& path) {
        File file(path, File::Read);
        return Array::loadFile(file);
    }
};

}
