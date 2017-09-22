#pragma once

#include <nms/core/view.h>
#include <nms/core/memory.h>
#include <nms/core/exception.h>

#ifndef NMS_LIST_DEBUG
#   define NMS_LIST_DEBUG   1
#endif


namespace nms
{

namespace io
{
class File;
class Path;
}

template<class T>
struct IList: public View<T>
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

    constexpr IList(Tsize size) : base() {
        base::capacity_ = (size+63)/64*64;
        base::data_     = mnew<T>(base::capacity_);
    }

public:
    ~IList() {
        this->clear();
        if (base::capacity_ == 0) {
            if (base::data_ != nullptr) {
                mdel(base::data_);
            }
            base::data_ = nullptr;
        }
    }

    IList(IList&& rhs)        = delete;
    IList(const IList& rhs)   = delete;

    IList& operator=(IList&& rhs)         = delete;
    IList& operator=(const IList& rhs)    = delete;

    operator View<const Tdata>() const noexcept {
        return { base::data_, base::size_ };
    }
#pragma endregion

#pragma region method
    /*! reserve storage */
    void reserve(Tsize newcnt) {
        if (newcnt > base::capacity_) {
            NMS_THROW(out_of_range(0u, base::capacity_, newcnt));
        }
    }

    /*! resize element count */
    void _resize(Tsize newcnt) {
        static_assert($is<$pod, Tdata>, "nms.List.resize: Tdata shold be POD type");
        base::size_ = newcnt;
    }

    /*! clear all elements */
    void clear() {
        // clear
        for (Tsize i = 0; i < base::size_; ++i) {
            base::data_[i].~Tdata();
        }
        base::size_ = 0;
    }

#pragma endregion

#pragma region operator=
    template<class U, class = $when_is<Tdata, U> >
    IList& operator=(const View<const U>& rhs) {
        clear();
        appends(rhs.data(), rhs.count());
        return *this;
    }

    template<class U, class = $when_is<Tdata, U> >
    IList& operator=(const View<U>& rhs) {
        clear();
        appends(rhs.data(), rhs.count());
        return *this;
    }

    template<u32 SN>
    IList& operator=(const Tdata(&rhs)[SN]) {
        clear();
        appends(View<const Tdata>{rhs});
        return *this;
    }
#pragma endregion

#pragma region operator+=
    template<class U, class = $when_is<Tdata, U> >
    IList& operator+=(const View<const U>& rhs) {
        appends(rhs.data(), rhs.count());
        return *this;
    }

    template<class U, class = $when_is<Tdata, U> >
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

#pragma region append
    /*! append elements to the end */
    template<class ...U>
    IList& appends(Tsize cnt, U&& ...u) {
    #if NMS_LIST_DEBUG
        if (base::size_ + cnt > base::capacity_) {
            NMS_THROW(out_of_range(0u, base::capacity_, base::size_ + cnt));
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
    #if NMS_LIST_DEBUG
        if (base::size_ + cnt > base::capacity_) {
            NMS_THROW(out_of_range(0u, base::capacity_, base::size_ + cnt));
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

    /*! append elements to the end */
    template<class ...U>
    IList& append(U&& ...u) {
        appends(1, fwd<U>(u)...);
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
        static_assert($is<$pod, Tdata>, "nms.IList.saveFile: should be POD type");
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
        static_assert($is<$pod, Tdata>, "nms.IList.loadFile: should be POD type");

        static const auto info_expect = base::info;

        Tinfo info_value;
        file.read(&info_value, 1);

        if (info_value != info_expect) {
            NMS_THROW(unexpect(info_expect, info_value));
        }

        typename base::Tdims dims;
        file.read(&dims, 1);

        list._resize(dims[0]);
        file.read(list.data(), dims[0]);
    }
};

template<class T>
using ListTrait =Tcond< Is<$copyable, T>::$value, $copyable, Tcond<Is<$moveable, T>::$value, $moveable, void> >;

template<class T, u32 Icapacity=0, class = ListTrait<T> >
class List;

template<class T, u32 Icapacity>
class List<T, Icapacity, $copyable>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

#pragma region constructor
    List() noexcept {
        base::data_     = reinterpret_cast<T*>(this->buff_);
        base::capacity_ = $capacity;
    }

    template<class ...U, class=$when_as<Tdata, U...> >
    explicit List(Tsize count, U&& ...us): List{} {
        base::appends(count, fwd<U>(us)...);
    }

    template<class U, class=$when_as<Tdata, const U&> >
    List(const View<U>& view) : List{} {
        base::appends(view.data(), view.count());
    }

    template<class U, class=$when_as<Tdata, const U&> >
    List(const View<const U>& view) : List{} {
        base::appends(view.data(), view.count());
    }

    template<class U, u32 SN, class=$when_as<Tdata, const U&> >
    List(const U(&array)[SN]) : List{} {
        base::appends(View<const U>{array});
    }

    List(List&& rhs) noexcept : List{} {
        for (auto i = 0u; i < rhs.size_; ++i) {
            new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
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
        if (this == &rhs) {
            return *this;
        }

        base::clear();
        for (auto i = 0u; i < rhs.size_; ++i) {
            new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
        }
        rhs.View<Tdata>::operator=(View<Tdata>{});
        return *this;
    }

    List& operator=(const List& rhs) {
        base::clear();
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

protected:
    alignas(T) char buff_[sizeof(Tdata)*$capacity];
};

template<class T, u32 Icapacity>
class List<T, Icapacity, $moveable>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

#pragma region constructor
    List() noexcept {
        base::data_     = reinterpret_cast<T*>(this->buff_);
        base::capacity_ = $capacity;
    }

    template<class ...U, class=$when_as<Tdata, U...> >
    explicit List(Tsize count, U&& ...us) : List{} {
        base::appends(count, fwd<U>(us)...);
    }

    List(List&& rhs) noexcept : List{} {
        for (auto i = 0u; i < rhs.size_; ++i) {
            new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
        }
        rhs.View<Tdata>::operator=(View<Tdata>{});
    }

    List& operator=(List&& rhs) {
        if (this == &rhs) {
            return *this;
        }

        this->clear();
        for (auto i = 0u; i < rhs.size_; ++i) {
            new(&base::data_[base::size_++])Tdata(static_cast<Tdata&&>(rhs.data_[i]));
        }

        rhs.View<Tdata>::operator=(View<Tdata>{});

        return *this;
    }

    List(const List&)               = delete;
    List& operator=(const List&)    = delete;
#pragma endregion

protected:
    alignas(T) char buff_[sizeof(Tdata)*$capacity];
};

template<class T, u32 Icapacity>
class List<T, Icapacity, void>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

    constexpr static Tsize $capacity = Icapacity;

 #pragma region constructor
     List() noexcept {
        base::data_     = reinterpret_cast<T*>(this->buff_);
        base::capacity_ = $capacity;
    }

    template<class ...U, class=$when_as<Tdata, U...> >
    explicit List(Tsize count, U&& ...us): List{} {
        base::appends(count, fwd<U>(us)...);
    }

    List(List&&)                    = delete;
    List(const List&)               = delete;
    List& operator=(List&&)         = delete;
    List& operator=(const List&)    = delete;
#pragma endregion

protected:
    alignas(T) char buff_[sizeof(Tdata)*$capacity];
};

template<class T>
class List<T, 0, $copyable>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

#pragma region constructor
    List()
    {}

    explicit List(Tsize size): base(size)
    {}

    ~List() {
        base::capacity_ = 0;
    }

    List(List&& rhs) noexcept {
        View<Tdata>::operator=(rhs);
        rhs.View<Tdata>::operator=(View<Tdata>{});
    }

    List(const List& rhs): List({ rhs.count() }) {
        base::appends(rhs.data(), rhs.count());
    }

    List dup() const {
        auto tmp(*this);
        return tmp;
    }

    List& operator=(List&& rhs) {
        if (this != &rhs) {
            // this -> tmp
            auto tmp(static_cast<List&&>(*this));
            (void)tmp;

            // rhs -> this
            new(this)List(static_cast<List&&>(rhs));
        }
        return *this;
    }

    List& operator=(const List& rhs) {
        base::clear();
        base::reserve(rhs.count());
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

template<class T>
class List<T, 0, $moveable>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

#pragma region constructor
    List()
    { }

    explicit List(Tsize size): base(size)
    {}

    ~List() {
        base::capacity_ = 0;
    }

    List(List&& rhs) noexcept {
        View<Tdata>::operator=(rhs);
        rhs.View<Tdata>::operator=(View<Tdata>{});
    }

    List& operator=(List&& rhs) {
        if (this != &rhs) {
            // this -> tmp
            auto tmp(static_cast<List&&>(*this));
            (void)tmp;

            // rhs -> this
            new(this)List(static_cast<List&&>(rhs));
        }
        return *this;
    }

    List(const List&)               = delete;
    List& operator=(const List&)    = delete;
#pragma endregion

};

template<class T>
class List<T, 0, void>: public IList<T>
{
public:
    using base  = IList<T>;
    using Tdata = typename base::Tdata;
    using Tsize = typename base::Tsize;

#pragma region constructor
    List()
    { }

    explicit List(Tsize size): base(size)
    {}

    ~List() {
        base::capacity_ = 0;
    }

    List(List&&)                    = delete;
    List(const List&)               = delete;
    List& operator=(List&&)         = delete;
    List& operator=(const List&)    = delete;
#pragma endregion

};

}
