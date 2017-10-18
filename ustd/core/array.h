#pragma once

#include <ustd/core/type.h>

namespace ustd
{

template<typename T, usize ...N>
struct array;

template<typename T, usize N>
struct array<T, N>
{
    T _data[N];

    // property[r]: data
    fn data() const noexcept -> T* {
        return _data;
    }

    // property[r]: len
    static fn len() noexcept -> u32 {
        return N;
    }

    // method: access
    fn operator[](u32 idx) const noexcept -> T& {
        return _data[idx];
    }

    // iterator[const]: begin
    fn begin() const noexcept -> T* {
        return _data;
    }

    // iterator[const]: cend
    fn end() const noexcept -> T* {
        return _data + N;
    }
};

}
