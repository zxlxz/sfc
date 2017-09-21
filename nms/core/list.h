#pragma once

#include <nms/core/view.h>
#include <nms/core/memory.h>
#include <nms/core/exception.h>

namespace nms
{

namespace io
{
class File;
class Path;
}

#if defined(_DEBUG) && !defined(NDEBUG)
#   define NMS_LIST_DEBUG
#endif

template<class T>
class IList: public View<T>
{
public:
    using base  = View<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;
    using Tinfo = typename base::Tinfo;

#pragma region constructor
protected:
    constexpr IList() noexcept: base()
    {}

public:
    IList(IList&& rhs)        = delete;
    IList(const IList& rhs)   = delete;

    IList& operator=(IList&& rhs)         = delete;
    IList& operator=(const IList& rhs)    = delete;

    operator View<const Tdata>() const noexcept {
        return { base::data_, base::size_ };
    }
#pragma endregion

#pragma region method
    void reserve(Tsize newcnt) {
        if (newcnt > base::capacity_) {
            NMS_THROW(EOutOfRange<Tsize>(0, base::capacity_, newcnt));
        }
    }

    void resize(Tsize newcnt) {
        static_assert($is<$pod, Tdata>, "nms.List.resize: Tdata shold be POD type");
        base::size_ = newcnt;
    }

    void clear() {
        // clear
        for (Tsize i = 0; i < base::size_; ++i) {
            base::data_[i].~Tdata();
        }
        base::size_ = 0;
    }

#pragma endregion

#pragma region append
    /*! append elements to the end */
    template<class ...U>
    IList& append(U&& ...u) {
    #ifdef NMS_LIST_DEBUG
        if (base::size_+1 > base::capacity_) {
            NMS_THROW(EOutOfRange<Tsize>{0, base::capacity_, base::size_+1});
        }
    #endif
        new(&base::data_[base::size_++]) Tdata(fwd<U>(u)...);
        return *this;
    }

    /*! append elements to the end */
    template<class ...U>
    IList& appends(Tsize cnt, U&& ...u) {
    #ifdef NMS_LIST_DEBUG
        if (base::size_+cnt > base::capacity_) {
            NMS_THROW(EOutOfRange<Tsize>{0, base::capacity_, base::size_+cnt});
        }
    #endif
        for (Tsize i = 0; i < cnt; ++i) {
            new(&base::data_[base::size_++])Tdata(fwd<U>(u)...);
        }
        return *this;
    }

    /*! append(copy) elements to the end */
    template<class U>
    IList& appends(const U dat[], Tsize cnt) {
    #ifdef NMS_LIST_DEBUG
        if (base::size_+cnt > base::capacity_) {
            NMS_THROW(EOutOfRange<Tsize>{0, base::capacity_, base::size_+cnt});
        }
    #endif        
        for (Tsize i = 0; i < cnt; ++i) {
            new(&base::data_[base::size_++])Tdata(dat[i]);
        }
        return *this;
    }

    /*! append elements to the end */
    template<class U>
    IList& appends(const View<U>& view) {
        appends(view.data(), view.count());
        return *this;
    }
#pragma endregion

#pragma region operator+=
    template<class U, class = $when_as<Tdata, U> >
    IList& operator+=(const View<const U>& rhs) {
        appends(rhs.data(), rhs.count());
        return *this;
    }

    template<class U, class = $when_as<Tdata, U> >
    IList& operator+=(const View<U>& rhs) {
        appends(rhs.data(), rhs.count());
        return *this;
    }

    template<u32 SN>
    IList& operator+=(const Tdata(&rhs)[SN]) {
        appends(View<const Tdata>{rhs});
        return *this;
    }

    template<class U, class=$when_as<Tdata, U> >
    IList& operator+= (U&& u) {
        append(fwd<U>(u));
        return *this;
    }
#pragma endregion

#pragma region save/load
    void save(io::File& file) const {
        saveFile(*this, file);
    }

    static void load(IList& list, const io::File& file) {
        loadFile(list, file);
    }
#pragma endregion

protected:
    template<class File>
    static void saveFile(const IList& list, File& file) {
        const auto info = list.info();
        const auto dims = list.size();
        const auto data = list.data();
        const auto nums = list.count();
        file.write(&info, 1);
        file.write(&dims, 1);
        file.write(data, nums);
    }

    template<class File>
    static void loadFile(IList& list, const File& file) {

        Tinfo info;
        file.read(&info, 1);
        if (info != base::$info) {
            NMS_THROW(Eunexpect<Tinfo>(base::$info, info));
        }

        typename base::Tdims dims;
        file.read(&dims, 1);

        list.resize(dims[0]);
        file.read(list.data(), dims[0]);
    }
};

template<class T>
using ListTrait =Tcond< Is<$copyable, T>::$value, $copyable, Tcond<Is<$moveable, T>::$value, $moveable, void> >;

template<class T, u32 Icapacity>
struct ListBuffer
{
    constexpr static u32 $size = Icapacity;

protected:
    alignas(T) char buff_[Icapacity * sizeof(T)];
};

template<class T>
struct ListBuffer<T, 0>
{
    constexpr static u32 $size = 0;

protected:
    constexpr static T* buff_ = nullptr;
};

template<class T, u32 Icapacity, class = ListTrait<T> >
class List;

template<class T, u32 Icapacity>
class List<T, Icapacity, $copyable>
    : public    IList<T>
    , protected ListBuffer<T, Icapacity>
{
public:
    using base  = IList<T>;
    using Tbuff = ListBuffer<T, Icapacity>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

#pragma region constructor
    ~List() {
        base::clear();
        if ($capacity == 0) {
            mdel(base::data_);
        }
    }

    List() noexcept {
        base::data_     = reinterpret_cast<T*>(Tbuff::buff_);
        base::capacity_ = $capacity;
    }

    template<class Isize, class=$when<$capacity==0, Isize> >
    explicit List(const Isize(&size)[1]) {
        base::data_     = mnew<T>(size[0]);
        base::capacity_ = size[0];
    }

    template<class ...U, class=$when<$capacity!=0 && $as<Tdata, U...> > >
    explicit List(Tsize count, U&& ...us) : List{} {
        base::appends(count, fwd<U>(us)...);
    }

    template<class U, class=$when_as<T, U>, class=$when<$capacity!=0, U> >
    List(const View<U>& view) : List{} {
        base::appends(view.data(), view.count());
    }

    template<class U, class=$when_as<T, const U>, class=$when<$capacity!=0, U> >
    List(const View<const U>& view) : List{} {
        base::appends(view.data(), view.count());
    }

    template<class U, u32 SN, class=$when<$capacity!=0, U> >
    List(const U(&array)[SN]) : List{} {
        base::appends(View<const U>{array});
    }

    List(List&& rhs) noexcept : List{} {
        if (Tbuff::$size == 0) {
            base::size_ = rhs.size_;
            base::data_ = rhs.data_;
        }
        else {
            for (auto i = 0u; i < rhs.size_; ++i) {
                new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
            }
        }
        rhs.View<Tdata>::operator=(View<Tdata>{});
    }

    List(const List& rhs): List{} {
        base::appends(rhs.data(), rhs.count());
    }

    List dup() const {
        auto tmp(*this);
        return tmp;
    }

    List& operator=(List&& rhs) {
        if (this != &rhs) {
            this->clear();
            for (auto i = 0u; i < rhs.size_; ++i) {
                new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
            }
            rhs.View<Tdata>::operator=(View<Tdata>{});
        }
        return *this;
    }

    List& operator=(const List& rhs) {
        base::appends(rhs.data(), rhs.count());
        return *this;
    }
#pragma endregion

#pragma region save/load
    static List load(const io::File& file) {
        List list;
        base::loadFile(list, file);
        return list;
    }
#pragma endregion

};

template<class T, u32 Icapacity>
class List<T, Icapacity, $moveable>
    : public    IList<T>
    , protected ListBuffer<T, Icapacity>
{
public:
    using base  = IList<T>;
    using Tbuff = ListBuffer<T, Icapacity>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

#pragma region constructor
    ~List() {
        base::clear();
        if ($capacity == 0) {
            mdel(base::data_);
        }
    }

    List() noexcept {
        base::data_     = reinterpret_cast<T*>(Tbuff::buff_);
        base::capacity_ = $capacity;
    }

    template<class Isize, class=$when<$capacity==0, Isize> >
    explicit List(const Isize(&size)[1]) {
        base::data_     = mnew<T>(size[0]);
        base::capacity_ = size[0];
    }

    template<class ...U, class=$when<$capacity!=0> >
    explicit List(Tsize count, U&& ...us) : List{} {
        base::appends(count, fwd<U>(us)...);
    }

    List(List&& rhs) noexcept : List{} {
        if (Tbuff::$size == 0) {
            base::size_ = rhs.size_;
            base::data_ = rhs.data_;
        }
        else {
            for (auto i = 0u; i < rhs.size_; ++i) {
                new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
            }
        }
        rhs.View<Tdata>::operator=(View<Tdata>{});
    }

    List& operator=(List&& rhs) {
        if (this == &rhs) {
            return *this;
        }

        this->clear();
        if (Tbuff::$size == 0) {
            base::size_ = rhs.size_;
            base::data_ = rhs.data_;
        }
        else {
            for (auto i = 0u; i < rhs.size_; ++i) {
                new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
            }
        }
        rhs.View<Tdata>::operator=(View<Tdata>{});
        
        return *this;
    }

    List(const List& rhs)               = delete;
    List& operator=(const List& rhs)    = delete;
#pragma endregion

};

template<class T, u32 Icapacity>
class List<T, Icapacity, void>
    : public    IList<T>
    , protected ListBuffer<T, Icapacity>
{
public:
    using base  = IList<T>;
    using Tbuff = ListBuffer<T, Icapacity>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

 #pragma region constructor
     List() noexcept {
        base::data_     = reinterpret_cast<T*>(Tbuff::buff_);
        base::capacity_ = $capacity;
    }

    template<class Isize, class=$when<$capacity==0, Isize> >
    explicit List(const Isize(&size)[1]) {
        base::data_     = mnew<T>(size[0]);
        base::capacity_ = size[0];
    }

    template<class ...U, class=$when<$capacity!=0> >
    explicit List(Tsize count, U&& ...us): List{} {
        base::appends(count, fwd<U>(us)...);
    }

    List(List&& rhs)                = delete;
    List& operator=(List&& rhs)     = delete;
    List(const List& rhs)           = delete;
    List& operator=(const List& rhs)= delete;
#pragma endregion

};

}
