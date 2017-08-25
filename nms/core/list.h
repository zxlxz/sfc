#pragma once

#include <nms/core/view.h>
#include <nms/core/memory.h>

namespace nms
{

namespace io
{
class File;
class Path;
}

template<class T, u32 N, bool=$is_pod<T> >
struct ListBuff;

template<class T>
struct ListBuff<T, 0, true>
{
    constexpr operator T* () const noexcept {
        return nullptr;
    }
};

template<class T>
struct ListBuff<T, 0, false>
{
    constexpr operator T* () const noexcept {
        return nullptr;
    }
};

template<class T, u32 N>
struct ListBuff<T, N, true> {
    constexpr operator T* () const noexcept {
        return const_cast<T*>(buff_);
    }

    T buff_[N] = {};
};

template<class T, u32 N>
struct alignas(T) ListBuff<T, N, false>
{
    constexpr operator T* () const noexcept {
        return const_cast<T*>(reinterpret_cast<const T*>(buff_));
    }

    ubyte buff_[N*sizeof(T)] = {};
};

/* list: move able */
template<class T, u32 BuffSize=0>
class List
    : public View<T>
{
    using base = View<T>;

public:
    static constexpr u32 $BlockSize = 32;           // block  size
    static constexpr u32 $BuffSize = BuffSize;     // buff   size

public:
#pragma region constructor
    /*! constructor */
    constexpr List() noexcept
        : base{ nullptr,{ 0 },{ 1 } } {
        base::data_ = buff_;
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
        base::data_ = nullptr;
    }

    template<class ...Targs>
    List(u32 size, Targs&& ...args) 
        : List()
    {
        appends(size, fwd<Targs>(args)...);
    }

    /* move construct */
    List(List&& rhs) noexcept
        : base(rhs), buff_(rhs.buff_) {
        // check if using buff?
        if (rhs.data_ == rhs.buff_) {
            base::data_ = buff_;
        }

        // clear rhs
        rhs.data_ = rhs.buff_;
        rhs.size_[0] = 0;
    }

    List(const List& rhs) {
        appends(rhs.data(), rhs.count());
    }

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
        List tmp;
        tmp.appends(rhs.data(), rhs.count());
        *this = move(tmp);
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
        const auto count = (mem_size - sizeof(u32)) / sizeof(T);
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
    template<class ...U>
    List& reset(u32 n, U&& ...u) {
        List tmp(n, fwd<U>(u)...);
        nms::swap(*this, tmp);
        return *this;
    }

    /*!
     * append elements to the end
     */
    template<class ...U>
    List& _append(U&& ...u) {
        auto ptr = base::data();
        auto idx = base::size_[0]++;
        new(&ptr[idx])T(fwd<U>(u)...);
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
        _append(fwd<U>(u)...);
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
    * append(copy) elements to the end
    */
    template<class U>
    List& appends(const U* src, u32 cnt) {
        auto old_cnt = base::count();
        reserve(old_cnt + cnt);
        base::size_[0] += cnt;

        // modify data
        auto ptr = base::data_ + old_cnt;
        for (u32 i = 0; i < cnt; ++i, ++ptr) {
            new(ptr)T(src[i]);
        }

        return *this;
    }

    /*!
    * append an element to the end
    */
    template<class U>
    List& operator+=(U&& u) {
        append(fwd<U>(u));
        return *this;
    }
#pragma endregion

#pragma region save/load
    void save(io::File& os) const;
    void save(const io::Path& path) const;
    static List load(io::File& is);
    static List load(const io::Path& path);
#pragma endregion

protected:
    ListBuff<T, $BuffSize> buff_;
};

}
