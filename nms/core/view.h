#pragma once

#include <nms/core/base.h>
#include <nms/core/trait.h>

namespace nms
{
template<class T, u32 N = 0>
struct View;

using StrView = View<const char>;

template<class Tidx, class Tsize>
constexpr auto shrinkIdx(Tidx idx, Tsize size) {
    return idx >= 0 ? Tsize(idx) : size-Tsize(0-idx);
}

#pragma region mkStride
template<class Tint, u32 N, u32 ...I>
__forceinline constexpr auto mkStride(const Tint(&size)[N], U32<I...>) {
    return Vec<Tint,N>{ iprod(Seq<I>{}, size)... };
}

template<class Tint, u32 N>
__forceinline constexpr auto mkStride(const Tint(&size)[N]) {
    return mkStride(size, Seq<N>{});
}
#pragma endregion

template<class T, u32 N>
struct View
{
#pragma region defines
    constexpr static const auto $rank = N == 0 ? 1 : N;

    using u32xN = Vec<u32, $rank>;

    template<class U, u32 M>
    friend struct View;
#pragma endregion

#pragma region constructors
    /* default constructor */
    __forceinline View()  = default;

    /* default destructor */
    __forceinline ~View() = default;

    /*! construct view with data, size, stride */
    __forceinline constexpr View(T* data, const u32(&size)[$rank], const u32(&stride)[$rank])
        : data_{ data }, size_{ size }, stride_(stride)
    {}

    /*! construct view with data, size */
    __forceinline constexpr View(T* data, const u32(&size)[$rank])
        : data_{ data }, size_{ size }, stride_{ mkStride(size) }
    {}

    /*! convert to const View */
    __forceinline operator View<const T, N>() const noexcept {
        return { data_, size_, stride_ };
    }

#pragma endregion

#pragma region properties
    __forceinline static constexpr auto rank() {
        return $rank;
    }

    /*! get data pointer */
    __forceinline T* data() noexcept {
        return data_;
    }

    /*! get data pointer */
    __forceinline const T* data() const noexcept {
        return data_;
    }

    /*! get n-dim size */
    __forceinline auto size() const noexcept {
        return size_;
    }

    /*! get n-dim stride */
    __forceinline auto stride() const noexcept {
        return stride_;
    }

    /*! get total elements count */
    __forceinline auto count() const noexcept {
        return iprod(Seq<$rank>{}, size_);
    }

    /*!
     * get total elements count.
     * @see count
     */
    __forceinline u32 numel() const noexcept {
        return count();
    }

    /*!
     * get idx-dim size
     * @see size
     */
    __forceinline constexpr auto size(u32 idx) const noexcept {
        return size_[idx];
    }

    /*!
     * get idx-dim stride
     * @see stride
     */
    __forceinline constexpr auto stride(u32 idx) const noexcept {
        return stride_[idx];
    }

    /*!
     * test if stride is default
     * @see stride
     */
    __forceinline constexpr bool isNormal() const {
        return stride_ == mkStride(size_.data_);
    }

    /*!
     * test if empty (count()==0)
     * @see count
     */
    constexpr __forceinline bool isEmpty() const {
        return count() == 0;
    }

#pragma endregion

#pragma region access
    /*!
     * access specified element
     */
    template<class ...I>
    __forceinline const T& at(I ...idx) const noexcept {
        static_assert($all_is<$int,I...>,   "unexpect type");
        static_assert(sizeof...(I)==$rank,  "unexpect arguments count");
        return data_[offsets_of(idx...)];
    }

    /*!
     * access specified element
     */
    template<class ...I>
    __forceinline T& at(I ...idx) noexcept {
        static_assert($all_is<$int, I...>,  "unexpect type");
        static_assert(sizeof...(I)==$rank,  "unexpect arguments count");
        return data_[offsets_of(idx...)];
    }

    /*!
    * access specified element
    * @param idx indexs
    * @see at
    */
    template<class ...I>
    __forceinline decltype(auto) operator()(I ...idx) noexcept {
        return at(idx...);
    }

    /*!
     * access specified element
     * @param idx indexs
     * @see at
     */
    template<class ...I>
    __forceinline decltype(auto) operator()(I ...idx) const noexcept {
        return at(idx...);
    }

#pragma endregion

#pragma region slice
    /*! slice */
    __forceinline auto slice() const noexcept {
        return View<const T, $rank>{ data_, size_, stride_ };
    }

    /*! slice */
    __forceinline auto slice() noexcept {
        return View<T, $rank>{ data_, size_, stride_ };
    }

    /*! slice */
    template<class ...I, u32 ...S>
    __forceinline auto slice(const I(&...idxs)[S]) const noexcept {
        static_assert(all((S<=2)...),       "unexpect array size");
        return _slice(idxs...)._select(Index<(S > 1)...>{} );
    }

    /*! slice */
    template<class ...I, u32 ...S>
    __forceinline auto slice(const I(&...idxs)[S]) noexcept {
        static_assert(all((S<=2)...),       "unexpect array size");
        return _slice(idxs...)._select(Index<(S > 1)...>{});
    }


    /*!
     * slice the view
     * @param secs sections
     * @see slice
     */
    template<class ...I, u32 ...S >
    __forceinline auto operator()(const I(&...secs)[S]) noexcept {
        return slice(secs...);
    }

    /*!
     * slice the view
     * @param secs sections
     * @see slice
     */
    template<class ...I, u32 ...S >
    __forceinline auto operator()(const I(&...secs)[S]) const noexcept {
        return slice(secs...);
    }
#pragma endregion

#pragma region save/load
    __forceinline static u8x4 typeinfo() {
        const auto ch =
            $is<$uint, T> ? 'u' :
            $is<$sint, T> ? 'i' :
            $is<$float,T> ? 'f' :
            '?';

        const auto size = sizeof(T);
        const u8x4 val = { u8('$'), u8(ch), u8('0' + size), u8('0' + N) };

        return val;
    }

#pragma endregion

#pragma region when<N==0>
    /*!
     * construct: cstring -> StrView
     */
    template<u32 Usize, class = $when<Usize !=0 && N == 0> >
    __forceinline View(const char(&s)[Usize])
        : data_(s), size_{ Usize - 1 }, stride_{ 1 }
    {
        static_assert(N == 0, "StrView: unexpect arguments");
    }

    /*!
     * access specified element
     * when $rank == 1
     * @see at
     */
    template<class I, class = $when<sizeof(I) != 0 && N == 0> >
    __forceinline T& operator[](I idx) noexcept {
        return data_[idx >= 0 ? idx : I(size_[0]) - (0 - idx)];
    }

    /*!
     * access specified element
     * when $rank == 1
     * @see at
     */
    template<class I, class = $when<sizeof(I) != 0 && N == 0> >
    __forceinline const T& operator[](I idx) const noexcept {
        return data_[idx >= 0 ? idx : I(size_[0]) - (0 - idx)];
    }

    /*!
     * slice({begin, end})
     * when: N == 1
     */
    template<class I, class = $when<sizeof(I) != 0 && N == 0> >
    __forceinline auto slice(I begin, I end) noexcept {
        return _slice({ begin, end });
    }

    /*!
     * slice({begin, end})
     * when: N == 1
     */
    template<class I, class = $when<sizeof(I) != 0 && N == 0> >
    __forceinline auto slice(I begin, I end) const noexcept {
        return _slice({ begin, end });
    }

#pragma endregion

protected:
    T*      data_;
    u32xN   size_;
    u32xN   stride_;    // stride[0] = capicity

#pragma region offset_of
    template<u32 Idim>
    __forceinline constexpr auto offset_of(u32 idx) const noexcept {
        return idx * stride_[Idim];
    }

    template<u32 Idim>
    __forceinline constexpr auto offset_of(i32 idx) const noexcept {
        return (idx >= 0 ? idx : i32(size_[0]) + idx) * stride_[Idim];
    }

    template<u32 ...Idim, class ...Tidx>
    __forceinline constexpr auto offsets_of(U32<Idim...>, Tidx ...idx) const noexcept {
        return sum(offset_of<Idim>(idx)...);
    }

    template<class ...I>
    __forceinline constexpr auto offsets_of(I ...idxs) const noexcept {
        return offsets_of(Seq<$rank>{}, idxs...);
    }
#pragma endregion

#pragma region size_of
    template<u32 Idim, class Tidx>
    __forceinline constexpr u32 size_of(const Tidx(&)[1]) const noexcept {
        return 0;
    }

    template<u32 Idim, class Tidx>
    __forceinline constexpr u32 size_of(const Tidx(&idx)[2]) const noexcept {
        return shrinkIdx(idx[1], size_[Idim]) - shrinkIdx(idx[0], size_[Idim]) + 1;
    }

    template<u32 ...Idim, class ...Tidx, u32 ...Isize>
    __forceinline constexpr auto sizes_of(U32<Idim...>, const Tidx(&...idxs)[Isize]) const noexcept {
        return u32xN{size_of<Idim>(idxs)...};
    }

    template<class ...Tidx, u32 ...Isize>
    __forceinline constexpr auto sizes_of(const Tidx(&...idxs)[Isize]) const noexcept {
        return sizes_of(Seq<$rank>{}, idxs...);
    }
#pragma endregion

#pragma region _slice
    /* slice */
    template<class ...Tidx, u32 ...Isize>
    __forceinline View<T, N> _slice(const Tidx(&...s)[Isize]) noexcept {
        return { data_ + offsets_of(s[0]...), sizes_of(s...), stride_};
    }

    /* slice */
    template<class ...Tidx, u32 ...Isize>
    __forceinline constexpr View<const T, N> _slice(const Tidx(&...s)[Isize]) const noexcept {
        return { data_ + offsets_of(s[0]...), sizes_of(s...), stride_};
    }

    /* select dim */
    template<u32 ...I>
    __forceinline constexpr View<T, u32(sizeof...(I))> _select(U32<I...>) const noexcept {
        return { data_, { size_[I]... }, { stride_[I]... } };
    }
#pragma endregion
};

#pragma region iterator
template<class T>
auto begin(View<T>& v) {
    return v.data();
}

template<class T>
auto end(View<T>& v) {
    return v.data() + v.count();
}

template<class T>
auto begin(const View<T>& v) {
    return v.data();
}

template<class T>
auto end(const View<T>& v) {
    return v.data() + v.count();
}
#pragma endregion

#pragma region mkView
template<class T, u32 N, class Tsize, class Tstride>
__forceinline View<T, N> mkView(T* ptr, const Tsize(&size)[N], const Tstride(&stride)[N]) {
    return { ptr, size, stride };
}

template<class T, u32 N, class Tsize, class Tstride>
__forceinline View<T, N> mkView(T* ptr, const Tsize(&size)[N]) {
    return { ptr, size, mkStride(size) };
}

template<class T, u32 N>
__forceinline View<T, N> mkView(T* v, const u32(&len)[N]) {
    return { v, len };
}

#pragma endregion

/* view is lambda */
template<class T, u32 N>
__forceinline auto toLambda(const View<T,N>& val) {
    return val;
}

/* reshape */
template<class T, u32 N, u32 M>
__forceinline auto reshape(View<T, N> view, const u32(&new_size)[M]) {
    return mkView(view.dat(), new_size);
}

/* reshape */
template<class T, u32 N, u32 M>
__forceinline auto reshape(View<T, N> view, const u32(&new_size)[M], const u32(&new_stride)[M]) {
    return mkView(view.data(), new_size, new_stride);
}

/* premute */
template<class T, u32 N>
auto permute(const View<T, N> &view, const u32(&order)[N]) {
    u32 new_size[N];
    u32 new_stride[N];

    for (u32 i = 0; i < N; ++i) {
        new_size[i]     = view.size(order[i]);
        new_stride[i]   = view.stride(order[i]);
    }
    return mkView(const_cast<T*>(view.data()), new_size, new_stride);
}


/* check if view equals */
template<class T>
bool operator==(const View<T, 0>& a, const View<T, 0>& b) {
    if (&a == &b)               return true;
    if (a.size() != b.size())   return false;

    const auto size = a.size();
    for (u32 i = 0; i < size[0]; ++i) {
        if (a(i) != b(i)) {
            return false;
        }
    }
    return true;
}

/* check if view equals */
template<class T>
bool operator==(const View<T, 1>& a, const View<T, 1>& b) {
    if (&a == &b)               return true;
    if (a.size() != b.size())   return false;

	const auto size = a.size();
    for (u32 i = 0; i < size[0]; ++i) {
        if (a(i) != b(i)) {
            return false;
        }
    }
    return true;
}

/* check if view not equals */
template<class T, u32 N>
bool operator!=(const View<T, N>& a, const View<T, N>& b) {
    return !(a == b);
}

/* check if StrView equals */
template<u32 N>
static bool operator==(const StrView& a, const char(&b)[N]) {
    return a == StrView{ b };
}

/* check if StrView not equals */
template<u32 N>
static bool operator!=(const StrView& a, const char(&b)[N]) {
    return a != StrView{ b };
}


/* check if StrView equals */
template<u32 N>
static bool operator==(const char(&a)[N], const StrView& b) {
    return StrView{ a } == b;
}

/* check if StrView not equals */
template<u32 N>
static bool operator!=(const char(&a)[N], const StrView& b) {
    return StrView{ a } != b;
}

}
