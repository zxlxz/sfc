#pragma once

#include <nms/core/view.h>
#include <nms/io/file.h>

namespace nms::math
{

template<class T, u32 N=1>
class Array
    : public View<T, N>
    , public INocopyable
{
  public:
    using base  = View<T, N>;
    using u32xN = typename base::u32xN;

    Array(T* data, const u32(&len)[N])
        : base{ data, len }
        , deleter_{}
    {}

    template<class D>
    Array(T* data, const u32(&len)[N], D&& deleter)
        : base{ data, len }
        , deleter_(fwd<D>(deleter))
    {}

    explicit Array(const u32(&len)[N])
        : base(nullptr, len)
    {
        const auto n= base::count();
        if (n != 0) {
            auto dat    = mnew<T>(n);
            base::data_ = dat;
            deleter_    = delegate<void()>([=] { mdel(dat); });
        }
    }

    virtual ~Array() {
        // try delete
        if (base::data_ != nullptr) {
            if (deleter_) {
                deleter_();
            }
            base::data_ = nullptr;
        }
    }

    Array()
        : base{}
    {}

    Array(Array&& rhs) noexcept
        : base(static_cast<base&&>(rhs))
        , deleter_(move(rhs.deleter_))
    {
        rhs.data_ = nullptr;
    }

    Array& operator=(Array&& rhs) noexcept {
        base::operator  = (static_cast<base&&>(rhs));
        deleter_        = move(rhs.deleter_);
        return *this;
    }

    auto dup() const {
        Array tmp(base::size());
        tmp <<= *this;
        return tmp;
    }

    Array& resize(const u32(&newlen)[base::$rank]) {
        const auto oldlen = base::size();

        if (oldlen == newlen) {
            return *this;
        }

        Array tmp(newlen);
        *this = move(tmp);

        return *this;
    }

    Array& clear() {
        Array tmp;
        swap(*this, tmp);
        return *this;
    }

#pragma region save/load
    void save(io::File& os) const {
        const auto info = base::typeinfo();
        const auto size = base::size();

        os << info;
        os << size;
        os.write(base::data(), base::count());
    }

    void save(const io::Path& path) const {
        io::File file(path, io::File::Write);
        save(file);
    }

    static Array load(io::File& is) {
        u8x4        info;
        Vec<u32, N> size;

        is >> info;
        is >> size;
        if (info != base::typeinfo()) {
            throw_exception(EBadType{});
        }

        Array tmp(size);
        is.read(tmp.data(), tmp.count());
        return tmp;
    }

    static Array load(const io::Path& path) {
        io::File file(path, io::File::Read);
        auto ret = load(file);
        return ret;
    }
#pragma endregion

protected:
    Array(const Array& rhs)
        : Array(rhs.size()) {
        *this <<= rhs;
    }

private:
    delegate<void()>    deleter_;
};

}
