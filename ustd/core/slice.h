#pragma once

#include <ustd/core/type.h>

namespace ustd
{

template<typename T, usize N=0>
struct Slice;

template<typename T, usize N>
struct Slice
{
    T*  _data;
    u32 _dims[N];
    i32 _step[N];

    // constructor
    constexpr Slice() noexcept
        : _data{nullptr}, _dims{0}, _step{0}
    {}

    // constructor
    constexpr Slice(T* data, const u32(&dims)[N]) noexcept
        : Slice($index_seq<N>{}, data, dims)
    {}

    // constructor
    constexpr Slice(T* data, const u32(&dims)[N], const u32(&step)[N]) noexcept
        : Slice($index_seq<N>{}, data, dims, step)
    {}

    // property[r]: data
    fn data() noexcept -> T* {
        return _data;
    }

    // property[r]: rank
    static constexpr fn rank() noexcept -> usize {
        return N;
    }

    // property[r]: dims
    fn dims() const noexcept -> const u32(&)[N] {
        return _dims;
    }

    // property[r]: dims
    fn dims(u32 idx) const noexcept -> u32 {
        return _dims[idx];
    }

    // property[r]: step
    fn step() const noexcept -> const i32(&)[N] {
        return _step;
    }

    // property[r]: step
    fn step(u32 idx) const noexcept -> u32 {
        return _step[idx];
    }

    // property[r]: is_empty
    fn is_empty() -> bool {
        return iprod($index_seq<N>{}, _dims) == 0;
    }

    // operator: ()
    template<typename ...Tidx>
    fn operator()(Tidx ...idxs) const noexcept -> T& {
        return as_const(_data)[_offset_of($index_seq<N>{}, idxs...)];
    }

    // method: slice
    template<typename ...Tidx, usize ...Icnt>
    fn slice(const Tidx(&...s)[Icnt]) const noexcept {
        static_assert(all((Icnt <= 2)...), "unexpect array size");
        return _slice(s...)._select_dims($index_if<(Icnt!=0)...>{});
    }

private:
    /* construct: step=default */
    template<usize ...Idim>
    constexpr Slice($usize<Idim...>, T* data, const u32(&dims)[N])
        : _data{data}, _dims{dims[Idim]...}, _step{i32(vprod($usize<Idim>{}, dims))...}
    {}

    /* construct: */
    template<usize ...Idim>
    constexpr Slice($usize<Idim...>, T* data, const u32(&dims)[N], const u32(&step)[N])
        : _data{data}, _dims{dims[Idim]...}, _step{step[Idim]...}
    {}

    /* method: access */
    template<usize ...Idim, typename ...Tidx>
    fn _offset_of($usize<Idim...>, Tidx ...idxs) const -> i32 {
        return sum((_step[Idim]*idxs)...);
    }

    /* method: slice */
    template<usize ...Idim, typename ...Tidx, usize ...Icnt>
    fn _slice($usize<Idim...>, const Tidx(&...s)[Icnt]) const noexcept -> Slice<const T, N> {
        return { _data + _offset_of_dims($usize<Idim...>{}, s[0]...), { _size_of_dim<Idim>(s)...}, _step };
    }

    /* method: slice */
    template<usize ...Idim>
    fn _select_dims($usize<Idim...>) const noexcept -> Slice<T, sizeof...(Idim)> {
        return { _data, { _dims[Idim]... }, { _step[Idim]... } };
    }

    /* method: slice */
    template<usize Idim, typename Tidx>
    fn _offset_of_dim(Tidx idx) const noexcept -> u32 {
        return idx >= 0 ? u32(idx) : _dims[Idim]-u32(0-idx);
    }

    /* method: slice */
    template<usize ...Idim, typename ...Tidx>
    fn _offset_of_dims($usize<Idim...>, Tidx ...idx) const noexcept -> u32 {
        return sum( (_offset_of_dim<Idim>(idx)*_step[Idim])...);
    }

    /* method: slice */
    template<usize Idim, typename Tidx, usize Icnt>
    fn _size_of_dim(const Tidx(&s)[Icnt]) const noexcept -> u32 {
        return _offset_of_dim<Idim>(s[Icnt-1])*_step[Idim] - _offset_of_dim<Idim>(s[0])*_step[Idim];
    }
};

template<typename T>
struct Slice<T, 0>
{
    T*  _data;
    u32 _size;
    u32 _capacity;

    // constructor:
    Slice() noexcept
        : _data{nullptr}, _size{0}, _capacity{0}
    {}

    // constructor:
    Slice(T* ptr, u32 length, u32 capacity)
        : _data{ptr}, _size{length}, _capacity{capacity}
    {}

    // convert: const
    operator Slice<const T>() const noexcept {
        return { as_const(_data), _size, _capacity };
    }

    // operator: []
    fn operator[](u32 i) const noexcept -> T& {
        return _data[i];
    }

    // method: slice
    template<class I>
    fn slice(I begin, I end) const noexcept -> Slice<T, 0> {
        let beg_pos = begin >= 0 ? u32(begin) : _size - u32(0 - begin);
        let end_pos = end   >= 0 ? u32(end  ) : _size - u32(0 - end  );
        return { _data + beg_pos, end_pos - beg_pos + 1, 0 };
    }

    // property[r]: data
    fn data() const noexcept -> T* {
        return _data;
    }

    // property[r]: len
    fn len() const noexcept -> u32 {
        return _size;
    }

    // property[r]: capacity
    fn capacity() const noexcept -> u32{
        return _capacity;
    }

    // iterator: begin
    fn begin() const noexcept -> T* {
        return _data;
    }

    // iterator: begin
    fn end() const noexcept -> T* {
        return _data+_size;
    }
};

using i8s = Slice<i8>;
using u8s = Slice<u8>;

using i16s = Slice<i16>;
using u16s = Slice<u16>;

using i32s = Slice<i32>;
using u32s = Slice<u32>;

using i64s = Slice<i64>;
using u64s = Slice<u64>;

using f32s = Slice<f32>;
using f32s = Slice<f32>;

}
