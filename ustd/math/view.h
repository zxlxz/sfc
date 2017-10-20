#pragma once

#include <ustd/core.h>

namespace ustd
{

template<typename T, usize N>
struct View
{
    using $MathVopView = View;    // trait

    static constexpr usize $rank = N;

    T*  _data;
    u32 _dims[N];
    u32 _step[N];

    // constructor
    constexpr View() noexcept
        : _data{ nullptr }, _dims{ 0 }, _step{ 0 }
    {}

    // constructor
    constexpr View(T* data, const u32(&dims)[N]) noexcept
        : View($index_seq<N>{}, data, dims)
    {}

    // constructor
    constexpr View(T* data, const u32(&dims)[N], const u32(&step)[N]) noexcept
        : View($index_seq<N>{}, data, dims, step)
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
    fn step() const noexcept -> const u32(&)[N] {
        return _step;
    }

    // property[r]: step
    fn step(u32 idx) const noexcept -> u32 {
        return _step[idx];
    }

    // property[r]: count
    fn count() const noexcept -> u32 {
        return iprod($index_seq<N>{}, _dims);
    }

    // property[r]: is_empty
    fn is_empty() -> bool {
        return count() == 0;
    }

    // operator: ()
    template<typename ...Tidx>
    fn operator()(Tidx ...idxs) const noexcept -> T& {
        return as_const(_data)[_offset_of($index_seq<N>{}, idxs...)];
    }

    // method: View
    template<typename ...Tidx, usize ...Icnt>
    fn slice(const Tidx(&...s)[Icnt]) const noexcept {
        static_assert(all((Icnt <= 2)...), "unexpect array size");
        return _View(s...)._select_dims($index_if<(Icnt != 0)...>{});
    }

    // method: sfmt
    fn sfmt(string::String& outbuf, const fmt::Formatter& fmt) const -> void;

  private:
    /* construct: step=default */
    template<usize ...Idim>
    constexpr View($usize<Idim...>, T* data, const u32(&dims)[N])
        : _data{ data }, _dims{ dims[Idim]... }, _step{ 1, iprod($usize<Idim-1>{}, dims)... }
    {}

    /* construct: */
    template<usize ...Idim>
    constexpr View($usize<Idim...>, T* data, const u32(&dims)[N], const u32(&step)[N])
        : _data{ data }, _dims{ dims[Idim]... }, _step{ step[Idim]... }
    {}

    /* method: access */
    template<usize ...Idim, typename ...Tidx>
    fn _offset_of($usize<Idim...>, Tidx ...idxs) const -> u32 {
        return sum((_step[Idim] * idxs)...);
    }

    /* method: View */
    template<usize ...Idim, typename ...Tidx, usize ...Icnt>
    fn _View($usize<Idim...>, const Tidx(&...s)[Icnt]) const noexcept -> View<const T, N> {
        return { _data + _offset_of_dims($usize<Idim...>{}, s[0]...),{ _size_of_dim<Idim>(s)... }, _step };
    }

    /* method: View */
    template<usize ...Idim>
    fn _select_dims($usize<Idim...>) const noexcept -> View<T, sizeof...(Idim)> {
        return { _data,{ _dims[Idim]... },{ _step[Idim]... } };
    }

    /* method: View */
    template<usize Idim, typename Tidx>
    fn _offset_of_dim(Tidx idx) const noexcept -> u32 {
        return idx >= 0 ? u32(idx) : _dims[Idim] - u32(0 - idx);
    }

    /* method: View */
    template<usize ...Idim, typename ...Tidx>
    fn _offset_of_dims($usize<Idim...>, Tidx ...idx) const noexcept -> u32 {
        return sum((_offset_of_dim<Idim>(idx)*_step[Idim])...);
    }

    /* method: View */
    template<usize Idim, typename Tidx, usize Icnt>
    fn _size_of_dim(const Tidx(&s)[Icnt]) const noexcept -> u32 {
        return _offset_of_dim<Idim>(s[Icnt - 1])*_step[Idim] - _offset_of_dim<Idim>(s[0])*_step[Idim];
    }
};

template<class T>
fn _sfmt_view(string::String& outbuf, const fmt::Formatter& style, const View<T, 1>& v) -> void {
    let  nx = v.dims(0);

    outbuf.push_str("\n");
    for (mut ix = 0u; ix < nx; ++ix) {
        let val = v(ix);

        outbuf.push_str("    ");
        style.sfmt(outbuf, val);
        outbuf.push_str("\n");
    }
}

template<class T>
fn _sfmt_view(string::String& outbuf, const fmt::Formatter& style, const View<T, 2>& v) -> void {
    let nx = v.dims(0);
    let ny = v.dims(1);

    outbuf.push_str("\n");
    for (mut ix = 0u; ix < nx; ++ix) {
        outbuf.push_str("    ");
        for (mut iy = 0u; iy < ny; ++iy) {
            let val = v(ix, iy);

            style.sfmt(outbuf, val);
            outbuf.push_str(", ");
        }
        outbuf.push_str("\n");
    }
}

template<class T, usize N>
fn View<T, N>::sfmt(string::String& outbuf, const fmt::Formatter& style) const -> void {
    _sfmt_view(outbuf, style, *this);
}


}