#pragma once

#include <nms/core/type.h>
#include <nms/core/view.h>
#include <nms/core/memory.h>

namespace nms
{

template<class T, u32 N>
struct ListBuff;

template<class T>
struct ListBuff<T, 0>
{
    constexpr operator T* () const noexcept {
        return nullptr;
    }
};


template<class T, u32 N>
struct alignas(T) ListBuff
{
    constexpr operator T* () const noexcept {
        return (T*)(buff_);
    }

    Tcond<$is_pod<T>, T, ubyte[sizeof(T)]> buff_[N] = {};
};


/* list */
template<class T, u32 BuffSize=0>
class List
    : public View<T>
{
    using base = View<T>;

public:
    static constexpr u32 $BlockSize = 32;           // block  size
    static constexpr u32 $BuffSize  = BuffSize;     // buff   size

public:
#pragma region constructor
    /*! constructor */
    constexpr List() noexcept
        : base{ nullptr, {0}, {1} }
    {
        base::data_ = buff_;
    }

    List(const T* data, u32 count)
        : base{ nullptr, {0}, {1} }
    {
        base::data_ = buff_;
        appends(data, count);
    }


    List(const View<T>& v)
        : List{ v.data(), v.count() }
    {}

    List(const View<const T>& v)
        : List {v.data(), v.count() }
    {}

    template<u32 N>
    List(const T(&v)[N])
        : List {v, N }
    {}

    template<class ...U>
    List(u32 count, U&& ...args)
        : List{}
    {
        appends(count, fwd<U>(args)...);
    }

    /* destruct */
    ~List() {
        for (auto i = 0u; i < base::size_[0]; ++i) {
            base::data_[i].~T();
        }
        if (base::data_ != buff_) {
            mdel(base::data_);
        }
        base::size_[0] = 0;
        base::data_    = nullptr;
    }

    /* move construct */
    List(List&& rhs) noexcept
        : base(rhs), buff_(rhs.buff_)
    {
        // check if using buff?
        if (rhs.data_ == rhs.buff_) {
            base::data_ = buff_;
        }

        // clear rhs
        rhs.data_    = rhs.buff_;
        rhs.size_[0] = 0;
    }

    List(const List& rhs)
        : List(rhs.data(), rhs.count())
    {}

#pragma endregion

#pragma region operator=
    /* general assign */
    template<class U>
    List& operator=(U&& u) noexcept {
        List tmp(fwd<U>(u));
        nms::swap(*this, tmp);
        return *this;
    }

    /* move assign */
    List& operator=(List&& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        List tmp(move(*this));
        new (this)List(move(rhs));
        return *this;
    }

    /* copy assign */
    /* move assign */
    List& operator=(const List& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        List tmp(rhs.data(), rhs.count());
        *this = static_cast<List&&>(tmp);
        return *this;
    }
#pragma endregion

#pragma region property
    /* the number of elements that can be held in currently allocated storage */
    u32 capicity() const noexcept {
        if (base::data_ == buff_) {
            return $BuffSize;
        }
        const auto mem_size = msize(base::data_);
        const auto count    = (mem_size - sizeof(u32)) / sizeof(T);
        return u32(count);
    }

#pragma endregion

#pragma region method
    /* clear data */
    List& clear() {
        List tmp;
        swap(*this, tmp);
        return *this;
    }

    /*!
     * reserves storge
     * if (newlen <= capicity()) { do nothing }
     * else { new storge is allocated }
     */
    List& reserve(u32 newlen) {
        const auto oldcap = capicity();

        // do not need realloc
        if (newlen <= oldcap) {
            return *this;
        }

        const auto olddat = base::data();
        const auto oldlen = base::count();

        const auto newcap = (newlen + oldlen / 16 + 0x3Fu) & ~0x3Fu;
        const auto newdat = mnew<T>(newcap);

        if (oldlen > 0) {
            nms::mmov(newdat, olddat, oldlen);
            if (olddat != buff_) {
                nms::mdel(olddat);
            }
        }
        base::data_ = newdat;
        return *this;
    }

    /*!
     * change number of elements
     */
    template<class U>
    List& reset(u32 n, U&& u) {
        List tmp(n, fwd<U>(u));
        nms::swap(*this, tmp);
        return *this;
    }

    /*!
    * append elements to the end
    */
    template<class ...U>
    List& append(U&& ...u) {
        const auto oldlen = base::count();
        const auto newlen = oldlen + 1;
        reserve(newlen);
        base::size_[0] = newlen;

        auto ptr = base::data();
        new(&ptr[oldlen])T(fwd<U>(u)...);
        return *this;
    }

    /*!
     * append elements to the end 
     */
    template<class ...U>
    List& appends(u32 cnt, U&& ...u) {
        const auto oldlen = base::count();
        const auto newlen = oldlen + cnt;
        reserve(newlen);
        base::size_[0] = newlen;

        auto ptr = base::data();
        for (auto i = oldlen; i < newlen; ++i) {
            new(&ptr[i])T(fwd<U>(u)...);
        }
        return *this;
    }

    /*!
     * append elements to the end
     */
    List& appends(const T* ext, u32 ext_cnt) {
        auto old_cnt = base::count();
        reserve(old_cnt + ext_cnt);
        for (u32 i = 0; i < ext_cnt; ++i) {
            new(&base::data_[old_cnt + i])T(ext[i]);
        }
        base::size_[0] += ext_cnt;
        return *this;
    }

    /*!
     * append an element to the end
     */
    template<class U>
    auto operator+=(U&& u) {
        append(fwd<U>(u));
        return *this;
    }
#pragma endregion

protected:
    ListBuff<T, $BuffSize> buff_;
};

}
