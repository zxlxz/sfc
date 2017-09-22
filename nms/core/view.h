#pragma once

#include <nms/core/vec.h>
#include <nms/core/trait.h>

namespace nms
{

struct ViewInfo
{
    char mask;
    char type;
    char size;
    char rank;

    friend bool operator==(const ViewInfo& a, const ViewInfo& b) {
        return a.mask == b.mask && a.type == b.type && a.size == b.size && a.rank == b.rank;
    }

    friend bool operator!=(const ViewInfo& a, const ViewInfo& b) {
        return !(a==b);
    }

    NMS_API void format(IString& buf) const;
};


namespace
{
template<class T, u32 N>
constexpr static ViewInfo mk_viewinfo() {
    return { '$',
             $is<$uint, T> ? 'u' : $is<$sint, T> ? 'i' : $is<$float, T> ? 'f' : '?',
             char('0' + sizeof(T)),
             char('0' + N)
    };
}
}

template<class T, u32 N>
struct View
{
#pragma region defines
    static constexpr const auto $rank = N;

    using Tdata     = T;
    using Tthis     = View;
    using Trank     = u32;
    using Tsize     = u32;
    using Tstep     = i32;
    using Tdims     = Vec<Tsize, $rank>;
    using Tidxs     = Vec<Tstep, $rank>;

    using Tinfo     = ViewInfo;
    using Tview     = View;

    template<class U, u32 M>
    friend struct View;

#pragma endregion

#pragma region constructors
    /*! default constructor */
    constexpr View() noexcept
        : data_{nullptr}, size_{0}, step_{0}
    { }

    /*! default destructor */
    ~View() = default;

    /*! construct view with data, size, step */
    constexpr View(Tdata* data, const Tsize(&size)[$rank], const Tstep(&step)[$rank]) noexcept
        : data_{ data }, size_{ size }, step_(step)
    {}

    /*! construct view with data, size */
    constexpr View(Tdata* data, const Tsize(&size)[$rank]) noexcept
        : data_{ data }, size_{ size }, step_{ mkstep(size) }
    {}

    /*! convert to const View */
    operator View<const Tdata, $rank>() const noexcept {
        return { data_, size_, step_ };
    }

#pragma endregion

#pragma region properties
    static constexpr auto rank() noexcept{
        return $rank;
    }

    /*! get data pointer */
    Tdata* data() noexcept {
        return data_;
    }

    /*! get data pointer */
    const Tdata* data() const noexcept {
        return data_;
    }

    /*! get n-dim size */
    Tdims size() const noexcept {
        return size_;
    }

    /*! get n-dim step */
    Tidxs step() const noexcept {
        return step_;
    }

    /*! get total elements count */
    Tsize count() const noexcept {
        return iprod(Tseq<$rank>{}, size_);
    }

    /*! get idx-dim size  @see size */
    template<class Tdim>
    constexpr Tsize size(Tdim dim) const noexcept {
        return size_[dim];
    }

    /*! get idx-dim step @see step */
    template<class Tdim>
    constexpr Tstep step(Tdim dim) const noexcept {
        return step_[dim];
    }

    /*! * test if empty (count()==0)  @see count */
    constexpr auto isEmpty() const noexcept {
        return count() == 0;
    }

    /*! get type-info */
    constexpr static Tinfo info() {
        return mk_viewinfo<Tdata, $rank>();
    }
#pragma endregion

#pragma region access
    /*!
     * access specified element
     */
    template<class ...Tidx>
    __forceinline const Tdata& at(Tidx ...idx) const noexcept {
        static_assert($all_is<$int,Tidx...>,    "unexpect type");
        static_assert(sizeof...(Tidx)==$rank,    "unexpect arguments count");
        return data_[offsets_of(Tseq<$rank>{}, idx...)];
    }

    /*!
     * access specified element
     */
    template<class ...Tidx>
    __forceinline Tdata& at(Tidx ...idx) noexcept {
        static_assert($all_is<$int, Tidx...>,  "unexpect type");
        static_assert(sizeof...(Tidx)==$rank,  "unexpect arguments count");
        return data_[offsets_of(Tseq<$rank>{}, idx...)];
    }

    /*!
     * access specified element
     * @param idx indexs
     * @see at
     */
    template<class ...I>
    __forceinline Tdata& operator()(I ...idx) noexcept {
        return at(idx...);
    }

    /*!
     * access specified element
     * @param idx indexs
     * @see at
     */
    template<class ...I>
    __forceinline const Tdata& operator()(I ...idx) const noexcept {
        return at(idx...);
    }

#pragma endregion

#pragma region slice
    /*! slice */
    template<class ...Tidx, u32 ...Icnt, u32 Idim=Tcnt<(Icnt!=1)...>::$value>
    View<const Tdata, Idim> slice(const Tidx(&...ids)[Icnt]) const noexcept {
#ifndef NMS_CC_INTELLISENSE
        auto tmp = _slice(Tseq<$rank>{}, ids...);
        auto ret = tmp._select(Tindex<(Icnt!=1)...>{});
        return ret;
#else
        return {};
#endif
    }

    /*! slice */
    template<class ...Tidx, u32 ...Icnt, u32 Idim=Tcnt<(Icnt!=1)...>::$value>
    View<Tdata, Idim> slice(const Tidx(&...ids)[Icnt]) noexcept {
#ifndef NMS_CC_INTELLISENSE
        auto tmp = _slice(Tseq<$rank>{}, ids...);
        auto ret = tmp._select(Tindex<(Icnt!=1)...>{});
        return ret;
#else
        return {};
#endif
    }

    /*!
     * slice the view
     * @param ids sections
     * @see slice
     */
    template<class ...Tidx, u32 ...Icnt >
    auto operator()(const Tidx(&...ids)[Icnt]) const noexcept {
        return this->slice(ids...);
    }

    /*!
     * slice the view
     * @param ids sections
     * @see slice
     */
    template<class ...Tidx, u32 ...Icnt >
    auto operator()(const Tidx(&...ids)[Icnt]) noexcept {
        return this->slice(ids...);
    }

#pragma endregion

#pragma region methods
    View permute(const u32(&order)[$rank]) const noexcept {
        Tdims   new_size;
        Tidxs   new_step;

        for (Trank i = 0; i < $rank; ++i) {
            new_size[i] = this->size_[order[i]];
            new_step[i] = this->step_[order[i]];
        }

        return { data_, new_size, new_step };
    }

    template<u32 M>
    View<Tdata, M> reshape(const u32(&new_size)[M]) const noexcept {
        return { data_, new_size };
    }

    template<u32 M>
    View<Tdata, M> reshape(const u32(&new_size)[M], const u32(&new_step)[M]) const noexcept {
        return { data_, new_size, new_step };
    }

#pragma endregion

#pragma region save/load
#pragma endregion

protected:
    Tdata*  data_;
    Tdims   size_;
    Tidxs   step_;

#pragma region index_of
    template<u32 Idim, class Tidx>
    __forceinline constexpr Tsize index_of(Tidx idx) const noexcept {
        return idx >= 0 ? Tsize(idx) : size_[Idim] - Tsize(0 - idx);
    }
#pragma endregion

#pragma region offset_of
    template<u32 Idim, class Tidx>
    __forceinline constexpr Tstep offset_of(Tidx idx) const noexcept {
        return Tstep(idx) * step_[Idim];
    }

    template<u32 ...Idim, class ...Tidx>
    __forceinline constexpr Tstep offsets_of(Tu32<Idim...>, Tidx ...idx) const noexcept {
        return sum(offset_of<Idim>(idx)...);
    }
#pragma endregion

#pragma region size_of
    template<u32 Idim, class Tidx>
    Tsize size_of(const Tidx(&)[1]) const noexcept {
        return Tsize(1u);
    }

    template<u32 Idim, class Tidx>
    Tsize size_of(const Tidx(&idx)[2]) const noexcept {
        const Tsize s0 = index_of<Idim>(idx[0]);
        const Tsize s1 = index_of<Idim>(idx[1]);
        return s1 - s0 + 1;
    }
#pragma endregion

#pragma region _slice
    /* slice */
    template<u32 ...Idim, class ...Tidx, u32 ...Icnt>
    View<Tdata, $rank> _slice(Tu32<Idim...>, const Tidx(&...s)[Icnt]) noexcept {
        static_assert($all<(Icnt <= 2)...>, "unexpect array size");
        return{ data_ + offsets_of(Tu32<Idim...>{}, s[0]...), {size_of<Idim>(s)...}, step_ };
    }

    /* slice */
    template<u32 ...Idim, class ...Tidx, u32 ...Icnt>
    View<const Tdata, $rank> _slice(Tu32<Idim...>, const Tidx(&...s)[Icnt]) const noexcept {
        static_assert($all<(Icnt <= 2)...>, "unexpect array size");
        return { data_ + offsets_of(Tu32<Idim...>{}, s[0]...), {size_of<Idim>(s)...}, step_ };
    }

    /* select dim */
    template<u32 ...I>
    auto _select(Tu32<I...>) const noexcept {
        return View<Tdata, u32(sizeof...(I))>{ data_, { size_[I]... }, { step_[I]... } };
    }
#pragma endregion

private:
    template<class Tdims>
    static constexpr Tidxs mkstep(const Tdims& size) noexcept {
        return mkstep(Tseq<$rank>{}, size);
    }

    template<u32 ...Idim, class Tdims>
    static constexpr Tidxs mkstep(Tu32<Idim...>, const Tdims& size) noexcept {
        return { Tstep(iprod(Tseq<Idim>{}, size))... };
    }
};

template<class T>
struct View<T, 0>
{
#pragma region defines
    constexpr static auto $rank = 1u;

    using Tdata = T;
    using Trank = u32;
    using Tsize = u32;
    using Tstep = i32;
    using Tinfo = ViewInfo;
    using Tdims = Vec<Tsize, $rank>;

    template<class U, u32 M>
    friend struct View;
#pragma endregion

#pragma region constructors
    /* default constructor */
    __forceinline constexpr View() noexcept
        : data_{nullptr}, size_{0}, capacity_{0}
    { }

    /* default destructor */
    __forceinline ~View()
    { }

    /*! construct view with data, size*/
    __forceinline constexpr View(Tdata* data, Tsize size) noexcept
        : data_{ data }, size_{ size }, capacity_{0}
    {}

    /*! construct view with data, size */
    __forceinline constexpr View(Tdata* data, const Tsize(&size)[$rank]) noexcept
        : data_{ data }, size_{ size[0] }, capacity_{0}
    {}

    /*! construct view with array */
    template<u32 Isize>
    __forceinline constexpr View(Tdata(&data)[Isize]) noexcept
        : data_{ data }, size_{ ($is<char, T> || $is<const char, T>) ? Isize - 1 : Isize }, capacity_{0}
    {}

#pragma endregion

#pragma region properties
    __forceinline static constexpr auto rank() noexcept {
        return $rank;
    }

    /*! get data pointer */
    __forceinline Tdata* data() noexcept {
        return data_;
    }

    /*! get data pointer */
    __forceinline constexpr const Tdata* data() const noexcept {
        return data_;
    }

    /*! get n-dim size */
    __forceinline constexpr Tdims size() const noexcept {
        return { size_ };
    }

    /*! get total elements count */
    __forceinline constexpr Tsize count() const noexcept {
        return size_;
    }

    /*! get capicity */
    __forceinline constexpr Tsize capacity() const noexcept {
        return capacity_;
    }

    /*! test if empty (count()==0) */
    __forceinline constexpr bool isEmpty() const {
        return count() == 0;
    }

    /*! get type-info */
    constexpr static Tinfo info() {
        return mk_viewinfo<Tdata, $rank>();
    }
#pragma endregion

#pragma region access
    /*! access specified element */
    template<class Tidx>
    __forceinline const Tdata& at(Tidx idx) const noexcept {
        return data_[idx];
    }

    /*! access specified element */
    template<class Tidx>
    __forceinline Tdata& at(Tidx idx) noexcept {
        return data_[idx];
    }

    /*! access specified element */
    template<class Tidx>
    __forceinline auto& operator()(Tidx idx) noexcept {
        return at(idx);
    }

    /*! access specified element */
    template<class Tidx>
    __forceinline auto& operator()(Tidx idx) const noexcept {
        return at(idx);
    }

    /*! access specified element */
    template<class Tidx>
    __forceinline auto& operator[](Tidx idx) noexcept {
        return at(idx);
    }

    /*! access specified element */
    template<class Tidx>
    __forceinline auto& operator[](Tidx idx) const noexcept {
        return at(idx);
    }
#pragma endregion

#pragma region iterator
    // iterator type
    using Titr = Tdata*;

    // const-iterator type
    using Kitr = const Tdata*;

    __forceinline Titr begin() {
        return data_;
    }

    __forceinline Titr end() {
        return data_ + size_;
    }

    __forceinline Kitr begin() const {
        return data_;
    }

    __forceinline Kitr end() const {
        return data_ + size_;
    }

    /*! contains */
    template<class U>
    bool contains(const U& val) const {
        const auto idx = find(val);
        return idx != end();
    }

    /*! find */
    template<class U>
    Titr find(const U& val) {
        for (auto itr = begin(); itr != end(); ++itr) {
            if (*itr == val) {
                return itr;
            }
        }
        return end();
    }

    /*! find */
    template<class U>
    Kitr find(const U& val) const {
        for (auto itr = begin(); itr != end(); ++itr) {
            if (*itr == val) {
                return itr;
            }
        }
        return end();
    }

    /*! replace */
    template<class Tsrc, class Tdst>
    void replace(const Tsrc& src, Tdst&& dst) {
        for (auto& element: *this) {
            if (element == src) {
                element = fwd<Tdst>(dst);
            }
        }
    }

#pragma endregion

#pragma region slice
    /*! slice */
    template<class Tidx>
    View<Tdata> slice(Tidx first, Tidx last) noexcept {
        return { data_ + index_of(first), size_of(first, last) };
    }

    /*! slice */
    template<class Tidx>
    View<const Tdata> slice(Tidx first, Tidx last) const noexcept {
        return { data_ + index_of(first), size_of(first, last) };
    }

    /*! slice */
    template<class Tidx>
    View<Tdata> operator()(Tidx first, Tidx last) noexcept {
        return slice(first, last);
    }

    /*! slice */
    template<class Tidx>
    View<const Tdata> operator()(Tidx first, Tidx last) const noexcept {
        return slice(first, last);
    }
#pragma endregion

#pragma region method
    int compare(const View<T>& b) const {
        auto& a = *this;

        if (&a == &b) {
            return 0;
        }

        const auto na = a.count();
        const auto nb = b.count();

        if (na != nb) {
            return na > nb ? +1 : -1;
        }

        for (Tsize i = 0; i < na; ++i) {
            if (a[i] != b[i]) {
                return a[i] > b[i] ? +1 : -1;
            }
        }
        return 0;
    }

    bool operator== (const View& v) const {
        return compare(v) == 0;
    }

    bool operator!= (const View& v) const {
        return compare(v) != 0;
    }

#pragma endregion

protected:
    Tdata*  data_;
    Tsize   size_;
    Tsize   capacity_;

    template<class Tidx>
    Tsize index_of(Tidx idx) const {
        return idx >= 0 ? Tsize(idx) : size_ - Tsize(0-idx);
    }

    template<class Tidx>
    Tsize size_of(Tidx i0 , Tidx i1) const {
        const auto s0 = index_of(i0);
        const auto s1 = index_of(i1);
        return s1 - s0 + 1;
    }
};

template<class T>
struct Scalar
{
    using Trank = u32;
    using Tsize = u32;
    using Tdata = T;
    using Tview = Scalar;

    constexpr static const auto $rank = 0;

    Scalar(const T& t)
        : t_(t) {
    }

    template<class I>
    Tsize size(I /*idx*/) const noexcept {
        return 1u;
    }

    template<class ...I>
    const T& operator()(I .../*idx*/) const noexcept {
        return t_;
    }

    template<class ...I>
    T& operator()(I .../*idx*/) noexcept {
        return t_;
    }
protected:
    T   t_;
};

#pragma region make
template<class T, u32 N>
View<T, N> mkView(T* ptr, const u32(&size)[N], const u32(&step)[N]) {
    return { ptr, size, step };
}

template<class T, u32 N>
View<T, N> mkView(T* ptr, const u32(&size)[N]) {
    return { ptr, size };
}

template<class T, u32 S>
View<T> mkView(T(&data)[S]) {
    return {data};
}

#pragma endregion

}
